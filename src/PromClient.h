#ifndef promclient_h
#define promclient_h

#include <PromLokiTransport.h>
#include "WriteRequest.h"
#include "ReadRequest.h"
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
    SendResult instantQuery(ReadRequest& req, char* query, uint16_t queryLen, uint16_t time = 0);
    SendResult rangeQuery(ReadRequest& req, char* query, uint16_t queryLen, uint16_t start, uint16_t end);

    void urlEncode(char* str, uint16_t len, char* output);

    char* errmsg;

protected:
    friend ReadRequest;

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
    SendResult _query(char* path, ReadRequest& req, char* query, uint16_t queryLen, uint16_t time, uint16_t start, uint16_t end);
};


#endif
