#include "WriteRequest.h"


WriteRequest::WriteRequest(uint32_t numSeries, uint32_t bufferSize) : _seriesCount(numSeries), _bufferSize(bufferSize) {
    _series = new TimeSeries * [numSeries];
}

WriteRequest::~WriteRequest() {
    delete[] _series;
}

void WriteRequest::setDebug(Stream& stream) {
    _debug = &stream;
}

bool WriteRequest::addTimeSeries(TimeSeries& series) {
    errmsg = nullptr;
    if (_seriesPointer >= _seriesCount) {
        errmsg = (char*)"cannot add series, max number of series have already been added.";
        return false;
    }

    _series[_seriesPointer] = &series;
    _seriesPointer++;
    return true;
}

uint32_t WriteRequest::getBufferSize() {
    return _bufferSize;
}

int16_t WriteRequest::toSnappyProto(uint8_t* output) {
    errmsg = nullptr;
    DEBUG_PRINT("Begin serialization: ");
    PRINT_HEAP();
    uint8_t buffer[_bufferSize];
    pb_ostream_t os = pb_ostream_from_buffer(buffer, sizeof(buffer));

    SeriesTuple st = SeriesTuple{
        series: _series,
        seriesCnt : _seriesPointer
    };

    prometheus_WriteRequest rw = {};
    rw.timeseries.arg = &st;
    rw.timeseries.funcs.encode = &callback_encode_timeseries;
    if (!pb_encode(&os, prometheus_WriteRequest_fields, &rw)) {
        DEBUG_PRINT("Error from proto encode: ");
        DEBUG_PRINTLN(PB_GET_ERROR(&os));
        errmsg = (char*)"Error creating protobuf, enable debug logging to see more details";
        return -1;
    }

    DEBUG_PRINT("Bytes used for serialization: ");
    DEBUG_PRINTLN(os.bytes_written);

    DEBUG_PRINT("After serialization: ");
    PRINT_HEAP();

    // for (uint16_t i = 0; i < os.bytes_written; i++)
    // {
    //     if (buffer[i] < 0x10)
    //     {
    //         DEBUG_PRINT(0);
    //     }
    //     DEBUG_PRINT(buffer[i], HEX);
    // }
    // DEBUG_PRINTLN();

    snappy_env env;
    snappy_init_env(&env);
    DEBUG_PRINT("After Compression Init: ");
    PRINT_HEAP();

    size_t len = snappy_max_compressed_length(os.bytes_written);
    DEBUG_PRINT("Required buffer size for compression: ");
    DEBUG_PRINTLN(len);

    if (len > _bufferSize) {
        errmsg = (char*)"WriteRequest bufferSize is too small and will be overun during compression! Enable debug logging to see required buffer size";
        return -1;
    }

    snappy_compress(&env, (char*)buffer, os.bytes_written, (char*)output, &len);
    snappy_free_env(&env);

    DEBUG_PRINT("Compressed Len: ");
    DEBUG_PRINTLN(len);

    DEBUG_PRINT("After Compression: ");
    PRINT_HEAP();

    return len;
}

bool WriteRequest::callback_encode_timeseries(pb_ostream_t* ostream, const pb_field_t* field, void* const* arg) {
    SeriesTuple* st = (SeriesTuple*)*arg;
    for (int i = 0; i < st->seriesCnt; i++) {
        if (!pb_encode_tag_for_field(ostream, field)) {
            return false;
        }
        TimeSeries* series = st->series[i];
        prometheus_TimeSeries ts = {};
        ts.labels.arg = series;
        ts.labels.funcs.encode = &callback_encode_labels;
        ts.samples.arg = series;
        ts.samples.funcs.encode = &callback_encode_samples;
        if (!pb_encode_submessage(ostream, prometheus_TimeSeries_fields, &ts)) {
            return false;
        }
    }
    return true;
}

bool WriteRequest::callback_encode_labels(pb_ostream_t* ostream, const pb_field_t* field, void* const* arg) {
    TimeSeries* series = (TimeSeries*)*arg;

    for (int i = 0; i < series->_numLabels; i++) {
        if (!pb_encode_tag_for_field(ostream, field)) {
            return false;
        }
        prometheus_Label labels = {};
        labels.name.arg = series->_labels[i]->key;
        labels.name.funcs.encode = &callback_encode_string;
        labels.value.arg = series->_labels[i]->val;
        labels.value.funcs.encode = &callback_encode_string;
        if (!pb_encode_submessage(ostream, prometheus_Label_fields, &labels)) {
            return false;
        }
    }
    return true;
}

bool WriteRequest::callback_encode_string(pb_ostream_t* ostream, const pb_field_t* field, void* const* arg) {
    char* s = (char*)*arg;

    if (!pb_encode_tag_for_field(ostream, field)) {
        return false;
    }
    if (!pb_encode_string(ostream, (const uint8_t*)s, strlen(s))) {
        return false;
    }
    return true;
}

bool WriteRequest::callback_encode_samples(pb_ostream_t* ostream, const pb_field_t* field, void* const* arg) {
    TimeSeries* series = (TimeSeries*)*arg;
    for (int i = 0; i < series->_batchPointer; i++) {
        if (!pb_encode_tag_for_field(ostream, field)) {
            return false;
        }
        prometheus_Sample s = {};
        s.timestamp = series->_batch[i]->tsMillis;
        s.value = series->_batch[i]->val;
        if (!pb_encode_submessage(ostream, prometheus_Sample_fields, &s)) {
            return false;
        }
    }
    return true;
}
