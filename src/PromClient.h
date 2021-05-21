#ifndef promclient_h
#define promclient_h

#include "WriteRequest.h"

class PromClient {
public:
    PromClient();
    ~PromClient();

    void setUrl(const char* url);
    void setUser(const char* user);
    void setPass(const char* pass);
    void setCert(const char* cert);
    void setWifiSsid(const char* wifiSsid);
    void setWifiPass(const char* wifiPass);
    void setApn(const char* apn);
    void setApnLogin(const char* apnLogin);
    void setApnPass(const char* apnPass);

    void setDebug(Stream& stream);

    void setClient(Client& client);
    Client* getClient();

    bool begin();
    bool send(WriteRequest& req);
    int64_t getTimeMillis();

protected:
    Stream* _debug = nullptr;
    Client* _client = nullptr;

    virtual bool _begin() = 0;
    virtual bool _send(char* entry, size_t len) = 0;
    virtual int64_t _getTimeMillis() = 0;

    const char* _url;
    const char* _user;
    const char* _pass;
    const char* _cert;
    const char* _wifiSsid;
    const char* _wifiPass;
    const char* _apn;
    const char* _apnLogin;
    const char* _apnPass;
};


#endif