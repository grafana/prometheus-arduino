#include "PromClient.h"

PromClient::PromClient() {}
PromClient::~PromClient() {}

void PromClient::setUrl(const char* url) {
    _url = url;
}
void PromClient::setPath(char* path) {
    _path = path;
}
void PromClient::setPort(uint16_t port) {
    _port = port;
}
void PromClient::setUser(const char* user) {
    _user = user;
}
void PromClient::setPass(const char* pass) {
    _pass = pass;
}
void PromClient::setUseTls(bool useTls) {
    _useTls = useTls;
}
void PromClient::setCerts(const br_x509_trust_anchor* myTAs, int myNumTAs) {
    _TAs = myTAs;
    _numTAs = myNumTAs;
}
void PromClient::setWifiSsid(const char* wifiSsid) {
    _wifiSsid = wifiSsid;
}
void PromClient::setWifiPass(const char* wifiPass) {
    _wifiPass = wifiPass;
}
void PromClient::setApn(const char* apn) {
    _apn = apn;
}
void PromClient::setApnLogin(const char* apnLogin) {
    _apnLogin = apnLogin;
}
void PromClient::setApnPass(const char* apnPass) {
    _apnPass = apnPass;
}
void PromClient::setNtpServer(char* ntpServer) {
    _ntpServer = ntpServer;
}

void PromClient::setDebug(Stream& stream) {
    _debug = &stream;
}

void PromClient::setClient(Client& client) {
    _client = &client;
}
Client* PromClient::getClient() {
    return _client;
}

bool PromClient::begin() {
    return _begin();
}

bool PromClient::send(WriteRequest& req) {
    uint8_t buff[512] = { 0 };
    size_t len = req.toSnappyProto(buff);
    return _send(buff, len);
}

bool PromClient::_send(uint8_t* entry, size_t len) {
    DEBUG_PRINTLN("Sending To Prometheus");
    _httpClient->beginRequest();
    _httpClient->post(_path);
    if (_user && _pass) {
        _httpClient->sendBasicAuth(_user, _pass);
    }
    _httpClient->sendHeader("Content-Type", "application/x-protobuf");
    _httpClient->sendHeader("Content-Length", len);
    _httpClient->sendHeader("Content-Encoding", "snappy");
    //FIXME correct user agent
    _httpClient->sendHeader("User-Agent: loki-arduino/0.1.0");
    _httpClient->beginBody();
    // Don't use the httpClient print method because it doesn't have an option to specify the length.
    _client->write(entry, len);
    _httpClient->endRequest();
    DEBUG_PRINTLN("Sent, waiting for response");
    int statusCode = _httpClient->responseStatusCode();
    if (statusCode/100 == 2) {
        DEBUG_PRINTLN("Prom Send Succeeded");
        // We don't use the _httpClient->responseBody() method both because it allocates a string
        // and also because it doesn't understand a 204 response code not having a content-length
        // header and will wait until a timeout for additional bytes.
        while (_client->available()) {
            char c = _client->read();
            DEBUG_PRINT(c);
        }
    }
    else {
        DEBUG_PRINT("Prom Send Failed with code: ");
        DEBUG_PRINT(statusCode);
        while (_client->available()) {
            char c = _client->read();
            DEBUG_PRINT(c);
        }
        return false;
    }
    return true;


}


int64_t PromClient::getTimeMillis() {
    return _getTimeMillis();
}