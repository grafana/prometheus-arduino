#ifndef promclient_h
#define promclient_h

#include "WriteRequest.h"
#include <ArduinoHttpClient.h>
#include <bearssl_x509.h>
#include "PromDebug.h"

static const char UserAgent[] PROGMEM = "prom-arduino/0.1.0";

class PromClient {
public:
    PromClient();
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
    void setUseTls(bool useTls);
    void setCerts(const br_x509_trust_anchor* myTAs, int myNumTAs);
    void setWifiSsid(const char* wifiSsid);
    void setWifiPass(const char* wifiPass);
    void setApn(const char* apn);
    void setApnLogin(const char* apnLogin);
    void setApnPass(const char* apnPass);
    void setNtpServer(char* ntpServer);

    void setDebug(Stream& stream);

    void setClient(Client& client);
    Client* getClient();

    bool begin();
    SendResult send(WriteRequest& req);
    int64_t getTimeMillis();

    const char* errmsg;

protected:
    Stream* _debug = nullptr;
    Client* _client = nullptr;
    HttpClient* _httpClient = nullptr;

    virtual bool _begin() = 0;
    virtual int64_t _getTimeMillis() = 0;
    virtual void _checkConnection() = 0;

    SendResult _send(uint8_t* entry, size_t len);

    const char* _url;
    char* _path;
    uint16_t _port;
    const char* _user;
    const char* _pass;
    bool _useTls;
    const br_x509_trust_anchor* _TAs;
    uint8_t _numTAs;
    const char* _wifiSsid;
    const char* _wifiPass;
    const char* _apn;
    const char* _apnLogin;
    const char* _apnPass;
    char* _ntpServer = "pool.ntp.org";
};


#endif