#ifndef readrequest_h
#define readrequest_h

#include "TimeSeries.h"
#include "PromDebug.h"
#include <ArduinoJson.h>

class ReadRequest {
public:
    ReadRequest(uint32_t numSeries, uint32_t bufferSize = 512);
    ~ReadRequest();

    void setDebug(Stream& stream);

    bool addTimeSeries(TimeSeries& series);

    uint32_t getBufferSize();

    int16_t fromHttpBody(uint8_t* input, size_t len);

    char* errmsg;

private:
    uint32_t _seriesCount = 0;
    uint32_t _bufferSize = 0;
    uint8_t* _buffer = nullptr;
    uint32_t _bufferPos = 0;
    Stream* _debug = nullptr;

    DynamicJsonDocument* _json = nullptr;
    TimeSeries** _series = nullptr;
    uint8_t _seriesPointer = 0;

    void urlEncode(char* str, uint16_t len, char* output);

};

#endif