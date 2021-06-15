#ifndef timeseries_h
#define timeseries_h

#include <Arduino.h>

struct LabelSet {
    char* key;
    char* val;
};

class TimeSeries {
public:
    TimeSeries(uint16_t batchSize, const char* name, const char* labels);
    ~TimeSeries();

    bool addSample(int64_t tsMillis, double val);
    void resetSamples();

    char* errmsg;

private:
    friend class WriteRequest;
    class Sample;
    class Label;

    char* _name;
    TimeSeries::Label** _labels = nullptr;
    uint8_t _numLabels = 0;

    TimeSeries::Sample** _batch = nullptr;
    uint16_t _batchSize = 0;
    uint16_t _batchPointer = 0;

};

class TimeSeries::Sample {
public:
    Sample();
    ~Sample();
    int64_t tsMillis = 0;
    double val = 0;
};

class TimeSeries::Label {
public:
    Label(char* key, char* val);
    ~Label();
    char* key;
    char* val;

};

#endif