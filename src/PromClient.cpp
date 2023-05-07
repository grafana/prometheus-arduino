#include "PromClient.h"

PromClient::PromClient() {};
PromClient::PromClient(PromLokiTransport& transport) : _transport(&transport) {};
PromClient::~PromClient() {
    if (_httpClient) {
        delete _httpClient;
    }
};

void PromClient::setUrl(const char* url) {
    _url = url;
};
void PromClient::setPath(char* path) {
    _path = path;
};
void PromClient::setPort(uint16_t port) {
    _port = port;
};
void PromClient::setUser(const char* user) {
    _user = user;
};
void PromClient::setPass(const char* pass) {
    _pass = pass;
};


void PromClient::setDebug(Stream& stream) {
    _debug = &stream;
};

void PromClient::setTransport(PromLokiTransport& transport) {
    _transport = &transport;
}

uint16_t PromClient::getConnectCount() {
    return _connectCount;
}


bool PromClient::begin() {
    errmsg = nullptr;
    if (!_url) {
        errmsg = (char*)"you must set a url with setUrl()";
        return false;
    }
    if (!_path) {
        errmsg = (char*)"you must set a path with setPath()";
        return false;
    }
    if (!_port) {
        errmsg = (char*)"you must set a port with setPort()";
        return false;
    }

    if (!_transport) {
        errmsg = (char*)"you must set a transport with setTransport() first";
        return false;
    }
    _client = _transport->getClient();

    // Create the HttpClient
    _httpClient = new HttpClient(*_client, _url, _port);
    _httpClient->setTimeout(15000);
    _httpClient->setHttpResponseTimeout(15000);
    _httpClient->connectionKeepAlive();
    return true;
};

PromClient::SendResult PromClient::send(WriteRequest& req) {
    errmsg = nullptr;
    //TODO this should probably not allocate on the stack
    uint8_t buff[req.getBufferSize()] = { 0 };
    int16_t len = req.toSnappyProto(buff);
    if (len <= 0) {
        errmsg = req.errmsg;
        return PromClient::SendResult::FAILED_DONT_RETRY;
    }
    return _send(buff, len);
}

PromClient::SendResult PromClient::_send(uint8_t* entry, size_t len) {
    DEBUG_PRINTLN("Sending To Prometheus");

    // Make a HTTP request:
    if (_client->connected()) {
        DEBUG_PRINTLN("Connection already open");
    }
    else {
        DEBUG_PRINTLN("Connecting...");
        if (!_client->connect(_url, _port)) {
            DEBUG_PRINTLN("Connection failed");
            if (_client->getWriteError()) {
                DEBUG_PRINT("Write error on client: ");
                DEBUG_PRINTLN(_client->getWriteError());
                _client->clearWriteError();
            }
            errmsg = (char*)"Failed to connect to server, enable debug logging for more info";
            return PromClient::SendResult::FAILED_RETRYABLE;
        }
        else {
            DEBUG_PRINTLN("Connected.")
            _connectCount++;
        }
    }

    // Do a lot of this manually to avoid sending headers and things we don't want to send
    // Use the ArduinoHttpClient to facilitate in places.
    _httpClient->beginRequest();
    _client->print("POST ");
    _client->print(_path);
    _client->println(" HTTP/1.1");
    _client->print("Host: ");
    _client->println(_url);
    _client->println("Content-Type: application/x-protobuf");
    _client->println("Content-Encoding: snappy");
    if (_user && _pass) {
        _httpClient->sendBasicAuth(_user, _pass);
    }
    _client->print("User-Agent: ");
    _client->println(PromUserAgent);
    _client->print("Content-Length: ");
    _client->println(len);
    _httpClient->beginBody();
    _client->write(entry, len);
    _client->println();


    DEBUG_PRINTLN("Sent, waiting for response");
    uint8_t waitAttempts = 0;
    // The default wait in responseStatusCode is 1s which means the minimum return is at least 1s if data
    // is not immediately available. So instead we will loop for data for the first second allowing us 
    // to check for data much quicker
    while (!_client->available() && waitAttempts < 10) {
        delay(100);
        waitAttempts++;
    }
    int statusCode = _httpClient->responseStatusCode();
    if (statusCode == HTTP_ERROR_TIMED_OUT) {
        errmsg = (char*)"Timed out connecting to Loki";
        return PromClient::SendResult::FAILED_RETRYABLE;
    }
    if (statusCode == HTTP_ERROR_INVALID_RESPONSE) {
        errmsg = (char*)"Invalid response from server, correct address and port?";
        return PromClient::SendResult::FAILED_RETRYABLE;
    }
    int statusClass = statusCode / 100;
    if (statusClass == 2) {
        DEBUG_PRINTLN("Prom Send Succeeded");
        // We don't use the _httpClient->responseBody() method both because it allocates a string
        // and also because it doesn't understand a 204 response code not having a content-length
        // header and will wait until a timeout for additional bytes.
        while (_client->available()) {
            char c = _client->read();
            DEBUG_PRINT(c);
        }
    }
    else if (statusClass == 4) {
        DEBUG_PRINT("Prom Send Failed with code: ");
        DEBUG_PRINTLN(statusCode);
        while (_client->available()) {
            char c = _client->read();
            DEBUG_PRINT(c);
        }
        errmsg = (char*)"Failed to send to prometheus, 4xx response";
        return PromClient::SendResult::FAILED_DONT_RETRY;
    }
    else {
        DEBUG_PRINT("Prom Send Failed with code: ");
        DEBUG_PRINTLN(statusCode);
        while (_client->available()) {
            char c = _client->read();
            DEBUG_PRINT(c);
        }
        errmsg = (char*)"Failed to send to prometheus, 5xx or unexpected status code";
        return PromClient::SendResult::FAILED_RETRYABLE;
    }
    return PromClient::SendResult::SUCCESS;
};

PromClient::SendResult PromClient::instantQuery(ReadRequest& req, char* query, uint16_t queryLen, uint16_t time) {
    errmsg = nullptr;
    return _query("/api/prom/api/v1/query", req, query, queryLen, time, 0, 0);
};

PromClient::SendResult PromClient::rangeQuery(ReadRequest& req, char* query, uint16_t queryLen, uint16_t start, uint16_t end) {
    errmsg = nullptr;
    return _query("/api/prom/api/v1/query_range", req, query, queryLen, 0, start, end);
}


uint16_t PromClient::encodedLength(char* str, uint16_t len)
{
    uint16_t encodedLen = 0;
    for (int i = 0; i < len; i++) {
        char c = str[i];
        if (isAlphaNumeric(c) || (c == '-') || (c == '.') || (c == '_') || (c == '~')) {
            encodedLen++;
        } else {
            encodedLen += 3;
        }
    }
    // Add one more byte for the null terminator
    return encodedLen+1;
}

void PromClient::urlEncode(char* str, uint16_t len, char* output){
String encoded;

    uint16_t outPos = 0;

    for (int i = 0; i < len; i++) {
        char c = str[i];

        const char HEX_DIGIT_MAPPER[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

        if (isAlphaNumeric(c) || (c == '-') || (c == '.') || (c == '_') || (c == '~')) {
            output[outPos++] = c;
        } else {
            output[outPos++] = '%';
            output[outPos++] = HEX_DIGIT_MAPPER[(c >> 4) & 0xf];
            output[outPos++] = HEX_DIGIT_MAPPER[(c & 0x0f)];
        }
    }

    output[outPos] = '\0';
};

PromClient::SendResult PromClient::_query(char* path, ReadRequest& req, char* query, uint16_t queryLen, uint16_t time, uint16_t start, uint16_t end) {
    DEBUG_PRINTLN("Querying Prometheus");

    // Make a HTTP request:
    if (_client->connected()) {
        DEBUG_PRINTLN("Connection already open");
    }
    else {
        DEBUG_PRINTLN("Connecting...");
        if (!_client->connect(_url, _port)) {
            DEBUG_PRINTLN("Connection failed");
            if (_client->getWriteError()) {
                DEBUG_PRINT("Write error on client: ");
                DEBUG_PRINTLN(_client->getWriteError());
                _client->clearWriteError();
            }
            errmsg = (char*)"Failed to connect to server, enable debug logging for more info";
            return PromClient::SendResult::FAILED_RETRYABLE;
        }
        else {
            DEBUG_PRINTLN("Connected.")
            _connectCount++;
        }
    }

    uint16_t encodedLen = PromClient::encodedLength(query, queryLen);
    char encodedQuery[encodedLen];
    PromClient::urlEncode(query, queryLen, encodedQuery);

    // Do a lot of this manually to avoid sending headers and things we don't want to send
    // Use the ArduinoHttpClient to facilitate in places.
    _httpClient->beginRequest();
    _client->print("GET ");
    _client->print(path);
    _client->print("?query=");
    _client->write((uint8_t*)encodedQuery, strlen(encodedQuery));
    if (time != 0) {
        _client->print("&time=");
        _client->print(time);
    }
    if (start != 0) {
        _client->print("&start=");
        _client->print(start);
    }
    if (end != 0) {
        _client->print("&end=");
        _client->print(end);
    }
    _client->println(" HTTP/1.1");
    _client->print("Host: ");
    _client->println(_url);
    // _client->println("Content-Type: application/json");
    if (_user && _pass) {
        _httpClient->sendBasicAuth(_user, _pass);
    }
    _client->print("User-Agent: ");
    _client->println(PromUserAgent);
    // _client->println();
    _httpClient->endRequest();


    DEBUG_PRINTLN("Sent, waiting for response");
    uint8_t waitAttempts = 0;
    // The default wait in responseStatusCode is 1s which means the minimum return is at least 1s if data
    // is not immediately available. So instead we will loop for data for the first second allowing us 
    // to check for data much quicker
    while (!_client->available() && waitAttempts < 10) {
        delay(100);
        waitAttempts++;
    }
    int statusCode = _httpClient->responseStatusCode();
    if (statusCode == HTTP_ERROR_TIMED_OUT) {
        errmsg = (char*)"Timed out connecting to Loki";
        return PromClient::SendResult::FAILED_RETRYABLE;
    }
    if (statusCode == HTTP_ERROR_INVALID_RESPONSE) {
        errmsg = (char*)"Invalid response from server, correct address and port?";
        return PromClient::SendResult::FAILED_RETRYABLE;
    }
    int statusClass = statusCode / 100;
    if (statusClass == 2) {
        DEBUG_PRINTLN("Prom Query Succeeded, reading response.");
        _httpClient->skipResponseHeaders();
        // TODO handle error better
        req.fromHttpStream(_client);
        // In case of error, read the response body to finish so we can process another connection.
        while (_client->available()) {
            char c = _client->read();
            DEBUG_PRINT(c);
        }
    }
    else if (statusClass == 4) {
        DEBUG_PRINT("Prom Query Failed with code: ");
        DEBUG_PRINTLN(statusCode);
        while (_client->available()) {
            char c = _client->read();
            DEBUG_PRINT(c);
        }
        errmsg = (char*)"Failed to query prometheus, 4xx response";
        return PromClient::SendResult::FAILED_DONT_RETRY;
    }
    else {
        DEBUG_PRINT("Prom Query Failed with code: ");
        DEBUG_PRINTLN(statusCode);
        while (_client->available()) {
            char c = _client->read();
            DEBUG_PRINT(c);
        }
        errmsg = (char*)"Failed to query prometheus, 5xx or unexpected status code";
        return PromClient::SendResult::FAILED_RETRYABLE;
    }
    return PromClient::SendResult::SUCCESS;
};
