#ifndef esp32client_h
#define esp32client_h

#if defined(ESP32)



#include "PromClient.h"
#include <WiFi.h>
#include <time.h>
#include <esp_sntp.h>
#include <SSLClient.h>

//static const char UserAgent[] PROGMEM = "loki-arduino/" LOKI_CLIENT_VERSION " (ESP32)";

class ESP32Client : public PromClient {
public:
    ESP32Client();
    ~ESP32Client();
protected:
    bool _begin();
    // bool _send(char* entry, size_t len);
    int64_t _getTimeMillis();

private:
    WiFiClient* _wifiClient;
    // HttpClient* _httpClient;
    void _connect();

};


#endif // ESP32

#endif