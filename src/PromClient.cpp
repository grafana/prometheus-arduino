#include "PromClient.h"

PromClient::PromClient(){}
PromClient::~PromClient(){}

void PromClient::setUrl(const char* url){
    _url = url;
}
void PromClient::setUser(const char* user){
    _user = user;
}
void PromClient::setPass(const char* pass){
    _pass = pass;
}
void PromClient::setCert(const char* cert){
    _cert = cert;
}
void PromClient::setWifiSsid(const char* wifiSsid){
    _wifiSsid = wifiSsid;
}
void PromClient::setWifiPass(const char* wifiPass){
    _wifiPass = wifiPass;
}
void PromClient::setApn(const char* apn){
    _apn = apn;
}
void PromClient::setApnLogin(const char* apnLogin){
    _apnLogin = apnLogin;
}
void PromClient::setApnPass(const char* apnPass){
    _apnPass = apnPass;
}

void PromClient::setDebug(Stream& stream) {
    _debug = &stream;
}

void PromClient::setClient(Client& client){
    _client = &client;
}
Client* PromClient::getClient(){}

bool PromClient::begin(){
    return _begin();
}

bool PromClient::send(WriteRequest& req){
    char buff[512] = {0};
    size_t len = req.toSnappyProto(buff);
    Serial.print("compressed len: ");
    Serial.println(len);
    Serial.print("string len: ");
    Serial.println(strlen(buff));
    return _send(buff, len);

}

int64_t PromClient::getTimeMillis(){
    return _getTimeMillis();
}