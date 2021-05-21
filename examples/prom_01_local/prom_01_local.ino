
#include <Prometheus.h>

WriteRequest req(2);
LabelSet series1[] = { {"foo","val"},{"bar","val"} };
TimeSeries ts1(5, "gauge1", series1, 2);
LabelSet series2[] = { {"foo","val2"},{"bar","val2"} };
TimeSeries ts2(5, "gauge2", series2, 2);

void setup() {
    Serial.begin(115200);
    //Serial.begin(9600);
    while (!Serial)
        delay(10);

    Serial.println("Starting");
    delay(1000);
    req.addTimeSeries(&ts1);
    req.addTimeSeries(&ts2);
    ts1.addSample(millis(), 12.2);
    delay(10);
    ts1.addSample(millis(), 12.3);
    delay(10);
    ts2.addSample(millis(), 22.2);
    delay(10);
    ts2.addSample(millis(), 22.3);
    char buff[512] = { 0 };
    uint16_t len = req.toSnappyProto(buff);
    




};



void loop() {

};