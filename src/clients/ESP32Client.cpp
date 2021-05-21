#include "ESP32Client.h"

#if defined(ESP32)

ESP32Client::ESP32Client() {

}
ESP32Client::~ESP32Client() {

}

bool ESP32Client::_begin() {
    WiFiClient* client = new WiFiClient;
    // if (_cert && _cert.length() > 0)
    // {
    //     client->setCACert(_cert.c_str());
    // }
    // else
    // {
    //     client->setInsecure();
    // }
    _wifiClient = client;

    _httpClient = new HTTPClient;
    _httpClient->setReuse(true);
    //_httpClient->setUserAgent(UserAgent);

    if (_user && _pass)
    {
        _httpClient->setAuthorization(_user, _pass);
    }

    DEBUG_PRINTLN("Connecting Wifi");

    _connect();

    DEBUG_PRINTLN("Setting up sntp and setting time from pool.ntp.org")

        sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "172.20.31.1");
    sntp_init();

    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET)
    {
        delay(1000);
        if (_debug) {

        }
        DEBUG_PRINT(".");
    }

    DEBUG_PRINTLN("Time set succesfully");
}

bool ESP32Client::_send(char* entry, size_t len) {
    DEBUG_PRINT("Entry ")
    DEBUG_PRINTLN(entry);
    if (WiFi.status() != WL_CONNECTED)
    {
        WiFi.disconnect();
        yield();
        _connect();
    }
    if (_wifiClient)
    {
        _httpClient->begin(_url);
        _httpClient->addHeader("Content-Type", "application/x-protobuf");
        _httpClient->addHeader("Content-Encoding", "snappy");
        int httpCode = _httpClient->POST(reinterpret_cast<uint8_t*>(entry), len);
        if (httpCode > 0) {
            DEBUG_PRINT("POST...  Code: ");
            DEBUG_PRINTLN(httpCode);
            if (httpCode >= 400)
            {
                if (_debug) {
                    _httpClient->writeToStream(_debug);
                }
            }
            DEBUG_PRINTLN();
        }
        else {
            DEBUG_PRINT("POST... Error: ");
            DEBUG_PRINTLN(_httpClient->errorToString(httpCode).c_str());
        }

        _httpClient->end();
    }
    else
    {
        DEBUG_PRINTLN("Uninitialized wifi client, did you call begin()?");
    }

}

void ESP32Client::_connect() {
    DEBUG_PRINT("Connecting to '");
    DEBUG_PRINT(_wifiSsid);
    DEBUG_PRINT("' ...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(_wifiSsid, _wifiPass);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        DEBUG_PRINT(".");
    }
    DEBUG_PRINTLN("connected");

    DEBUG_PRINT("IP address: ");
    DEBUG_PRINTLN(WiFi.localIP());
}

int64_t ESP32Client::_getTimeMillis() {
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    return (uint64_t)tv_now.tv_sec * 1000L + (uint64_t)tv_now.tv_usec / 1000;
}

#endif