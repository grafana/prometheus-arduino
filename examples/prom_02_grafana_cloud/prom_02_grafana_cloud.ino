
#include <bearssl_x509.h>
#include "config.h"
#include "certificates.h"
#include <Prometheus.h>

LabelSet job = { "job", "esp32-test" };
LabelSet host = { "host","esp32" };

Prometheus client;
WriteRequest req(2);
LabelSet series1[] = { job, host, {"bar","val"} };
TimeSeries ts1(5, "gauge1", series1, 3);
LabelSet series2[] = { job, host, {"bar","val2"} };
TimeSeries ts2(5, "gauge1", series2, 3);

int loopCounter = 0;

void setup() {
    Serial.begin(115200);
    //Serial.begin(9600);
    while (!Serial)
        delay(10);

    Serial.println("Starting");

    client.setUrl("prometheus-us-central1.grafana.net");
    client.setPath("/api/prom/push");
    client.setPort(443);
    client.setUser(GC_USER);
    client.setPass(GC_PASS);
    client.setUseTls(true);
    client.setCerts(TAs, TAs_NUM);
    client.setWifiSsid(WIFI_SSID);
    client.setWifiPass(WIFI_PASSWORD);
    client.setDebug(Serial);
    client.begin();

    req.addTimeSeries(ts1);
    req.addTimeSeries(ts2);
    req.setDebug(Serial);

};



void loop() {
    int64_t time;
    time = client.getTimeMillis();
    Serial.println(time);

    if (loopCounter > 2)
    {
        //send
        loopCounter = 0;
        if (!client.send(req)) {
            Serial.println(client.errmsg);
        }
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