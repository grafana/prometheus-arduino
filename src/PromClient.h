#ifndef promclient_h
#define promclient_h

#include <PromLokiTransport.h>
#include "WriteRequest.h"
#include <ArduinoHttpClient.h>
#include "PromDebug.h"

static const char PromUserAgent[] PROGMEM = "prom-arduino/0.2.2";

class PromClient {
public:
    PromClient();
    PromClient(PromLokiTransport& transport);
    ~PromClient();

    enum SendResult {
        SUCCESS,
        FAILED_RETRYABLE,
        FAILED_DONT_RETRY
    };

    void setUrl(const char* url);
    void setPath(char* path);
    void setPort(uint16_t port);
    void setUser(const char* user);
    void setPass(const char* pass);

    void setDebug(Stream& stream);

    void setTransport(PromLokiTransport& transport);
    uint16_t getConnectCount();

    bool begin();
    SendResult send(WriteRequest& req);

    char* errmsg;

protected:
    Stream* _debug = nullptr;
    PromLokiTransport* _transport = nullptr;
    Client* _client = nullptr;
    HttpClient* _httpClient = nullptr;

    const char* _url;
    char* _path;
    uint16_t _port;
    const char* _user;
    const char* _pass;
    uint16_t _connectCount = 0;

    SendResult _send(uint8_t* entry, size_t len);
};


#endif
