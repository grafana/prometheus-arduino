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
    TimeSeries(uint16_t batchSize, uint16_t nameLen, uint16_t labelKeyLen, uint16_t labelValLen, uint8_t numLabels);
    ~TimeSeries();

    class Sample;
    class Label;

    bool addSample(int64_t tsMillis, double val);
    void resetSamples();

    bool setLabel(uint8_t pos, char* key, uint16_t keyLen, char* val, uint16_t valLen);
    bool setName(const char* name, uint16_t nameLen);

    TimeSeries::Label* getLabel(uint8_t pos);

    char* errmsg;

private:
    friend class WriteRequest;
    
    char* _name;
    uint16_t _maxNameLen = 0;
    TimeSeries::Label** _labels = nullptr;
    uint8_t _numLabels = 0;
    uint8_t _labelsPointer = 0;

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
    Label(uint32_t keyLen, uint32_t valLen);
    ~Label();
    char* key;
    char* val;
    uint16_t _maxKeyLen = 0;
    uint16_t _maxValLen = 0;
};

#endif