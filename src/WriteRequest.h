#ifndef writerequest_h
#define writerequest_h

#include "TimeSeries.h"
#include "snappy/snappy.h"
#include "proto/pb.h"
#include "proto/pb_encode.h"
#include "proto/remote.pb.h"

class WriteRequest {
public:
    WriteRequest(int numSeries);
    ~WriteRequest();

    bool addTimeSeries(TimeSeries& series);

    uint16_t estimateProtoBuffSize();
    uint16_t toSnappyProto(char* output);

    const char* errmsg;

private:
    int _seriesCount = 0;

    TimeSeries** _series = nullptr;
    uint8_t _seriesPointer = 0;

    static bool callback_encode_timeseries(pb_ostream_t* ostream, const pb_field_t* field, void* const* arg);
    static bool callback_encode_labels(pb_ostream_t* ostream, const pb_field_t* field, void* const* arg);
    static bool callback_encode_samples(pb_ostream_t* ostream, const pb_field_t* field, void* const* arg);
    static bool callback_encode_string(pb_ostream_t* ostream, const pb_field_t* field, void* const* arg);

};

struct SeriesTuple {
    TimeSeries** series;
    uint8_t seriesCnt;
};


#endif