
#include "config.h"
#include <Prometheus.h>


Prometheus client;
WriteRequest req(2);
LabelSet series1[] = { {"foo","val"},{"bar","val"} };
TimeSeries ts1(5, "gauge1", series1, 2);
LabelSet series2[] = { {"foo","val2"},{"bar","val2"} };
TimeSeries ts2(5, "gauge2", series2, 2);

int loopCounter = 0;

void setup() {
    Serial.begin(115200);
    //Serial.begin(9600);
    while (!Serial)
        delay(10);

    Serial.println("Starting");

    client.setUrl("http://172.20.70.20:8080/api/v1/push");
    client.setWifiSsid(WIFI_SSID);
    client.setWifiPass(WIFI_PASSWORD);
    client.setDebug(Serial);
    client.begin();

    req.addTimeSeries(ts1);
    req.addTimeSeries(ts2);

};



void loop() {
    int64_t time;
    time = client.getTimeMillis();
    Serial.println(time);

    if (loopCounter > 2)
    {
        //send
        loopCounter = 0;
        client.send(req);
        ts1.resetSamples();
        ts2.resetSamples();
    }
    else
    {

        ts1.addSample(time, millis());
        ts2.addSample(time, millis() * 10);
        loopCounter++;
    }

    delay(500);


};