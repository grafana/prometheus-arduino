#ifndef writerequest_h
#define writerequest_h

#include "TimeSeries.h"
#include <snappy.h>
#include <pb.h>
#include "proto/remote.pb.h"
#include "PromDebug.h"

class WriteRequest {
public:
    WriteRequest(uint32_t numSeries, uint32_t bufferSize = 512);
    ~WriteRequest();

    void setDebug(Stream& stream);

    bool addTimeSeries(TimeSeries& series);
    
    int16_t toSnappyProto(uint8_t* output);

    int16_t fromSnappyProto(uint8_t* input, size_t len);

    uint32_t getBufferSize();

    char* errmsg;

private:
    uint32_t _seriesCount = 0;
    uint32_t _bufferSize = 0;
    uint8_t* _buffer = nullptr;
    uint32_t _bufferPos = 0;
    Stream* _debug = nullptr;

    TimeSeries** _series = nullptr;
    uint8_t _seriesPointer = 0;

    static bool callback_encode_timeseries(pb_ostream_t* ostream, const pb_field_t* field, void* const* arg);
    static bool callback_encode_labels(pb_ostream_t* ostream, const pb_field_t* field, void* const* arg);
    static bool callback_encode_samples(pb_ostream_t* ostream, const pb_field_t* field, void* const* arg);
    static bool callback_encode_string(pb_ostream_t* ostream, const pb_field_t* field, void* const* arg);

    static bool callback_decode_timeseries(pb_istream_t* istream, const pb_field_t* field, void** arg);
    static bool callback_decode_labels(pb_istream_t* istream, const pb_field_t* field, void** arg);
    static bool callback_decode_samples(pb_istream_t* istream, const pb_field_t* field, void** arg);
    static bool callback_decode_label_key(pb_istream_t* istream, const pb_field_t* field, void** arg);
    static bool callback_decode_label_val(pb_istream_t* istream, const pb_field_t* field, void** arg);

};

struct SeriesTuple {
    TimeSeries** series;
    uint8_t seriesCnt;
};


#endif