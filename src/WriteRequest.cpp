#include "WriteRequest.h"

WriteRequest::WriteRequest(uint32_t numSeries, uint32_t bufferSize)
    : _seriesCount(numSeries), _bufferSize(bufferSize) {
    _series = new TimeSeries*[numSeries];
    _buffer = new uint8_t[bufferSize];
}

WriteRequest::~WriteRequest() {
    delete[] _series;
    delete[] _buffer;
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

    SeriesTuple st = SeriesTuple{series : _series, seriesCnt : _seriesPointer};

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

int16_t WriteRequest::fromSnappyProto(uint8_t* input, size_t len) {
    DEBUG_PRINT("Begin deserialization: ");
    PRINT_HEAP();
    size_t uncompressedLen;
    bool cl = snappy_uncompressed_length((char*)input, len, &uncompressedLen);
    if (!cl) {
        errmsg = (char*)"Call to snappy_uncompressed_length failed";
        return -1;
    }
    DEBUG_PRINT("Required buffer size for decompression: ");
    DEBUG_PRINTLN(uncompressedLen);

    if (uncompressedLen > _bufferSize) {
        errmsg = (char*)"WriteRequest bufferSize is too small and will be overun during decompression! Enable debug logging to see required buffer size";
        return -1;
    }

    int8_t res = snappy_uncompress((char*)input, len, (char*)_buffer);
    DEBUG_PRINT("Decompression result: ");
    DEBUG_PRINTLN(res);
    if (res != 0) {
        errmsg = (char*)"Decompression failed, enable debug logging to see exit status and more info.";
        return -1;
    }

    SeriesTuple st = SeriesTuple{
        series : _series,
        seriesCnt : _seriesPointer,
    };

    pb_istream_t is = pb_istream_from_buffer(_buffer, uncompressedLen);
    prometheus_WriteRequest message = prometheus_WriteRequest_init_zero;
    message.timeseries.arg = &st;
    message.timeseries.funcs.decode = &callback_decode_timeseries;
    bool status = pb_decode(&is, prometheus_WriteRequest_fields, &message);
    if (!status) {
        DEBUG_PRINT("Error from proto encode: ");
        DEBUG_PRINTLN(PB_GET_ERROR(&is));
        return -1;
    }

    DEBUG_PRINT("End deserialization: ");
    PRINT_HEAP();
    return 0;
}

bool WriteRequest::callback_decode_timeseries(pb_istream_t* istream,
                                              const pb_field_t* field,
                                              void** arg) {
    SeriesTuple* st = (SeriesTuple*)*arg;

    uint8_t i = 0;
    while (istream->bytes_left) {
        Serial.println("Decoding timeseries");
        TimeSeries* series = st->series[i];
        series->_labelsPointer = 0;
        prometheus_TimeSeries ts = {};
        ts.labels.arg = series;
        ts.labels.funcs.decode = &callback_decode_labels;
        ts.samples.arg = series;
        ts.samples.funcs.decode = &callback_decode_samples;
        if (!pb_decode(istream, prometheus_TimeSeries_fields, &ts)) {
            return false;
        }
        i++;
        // We decode everything, but if there are more timeseries than we have
        // allocated, we overrwite the last one
        // TODO should we error here instead?
        if (i >= st->seriesCnt) {
            i = st->seriesCnt - 1;
        }
    }
    return true;
}

bool WriteRequest::callback_decode_labels(pb_istream_t* istream,
                                          const pb_field_t* field,
                                          void** arg) {
    TimeSeries* ts = (TimeSeries*)*arg;
    // Decode all the labels
    while (istream->bytes_left) {
        if (ts->_labelsPointer > ts->_numLabels) {
            ts->errmsg = (char*)"More labels being decoded than allocated for timeseries";
            return false;
        }
        prometheus_Label labels = {};
        labels.name.arg = ts;
        labels.name.funcs.decode = &callback_decode_label_key;
        labels.value.arg = ts;
        labels.value.funcs.decode = &callback_decode_label_val;
        if (!pb_decode(istream, prometheus_Label_fields, &labels)) {
            return false;
        }
        ts->_labelsPointer++;
    }

    return true;
}
bool WriteRequest::callback_decode_samples(pb_istream_t* istream,
                                           const pb_field_t* field,
                                           void** arg) {
    TimeSeries* series = (TimeSeries*)*arg;
    while (istream->bytes_left) {
        Serial.println("Decoding samples");
        prometheus_Sample s = {};
        if (!pb_decode(istream, prometheus_Sample_fields, &s)) {
            return false;
        }
        // series->addSample(s.timestamp, s.value);
        Serial.print("Timestamp: ");
        Serial.println(s.timestamp);
        Serial.print("Value: ");
        Serial.println(s.value);
    }
    return true;
}

bool WriteRequest::callback_decode_label_key(pb_istream_t* istream,
                                          const pb_field_t* field,
                                          void** arg) {
    TimeSeries* ts = (TimeSeries*)*arg;
    TimeSeries::Label* l = ts->_labels[ts->_labelsPointer];

    size_t bytesLeft = istream->bytes_left;

    if (l->_maxKeyLen < istream->bytes_left) {
        ts->errmsg = (char*)"Label key is longer than allocated buffer";
        return false;
    }

    if (!pb_read(istream, (unsigned char*)l->key, istream->bytes_left))
        return false;

    // We allocated enough space for the max length plus 1 for a terminator, probably.
    l->key[bytesLeft] = '\0';

    return true;
}

bool WriteRequest::callback_decode_label_val(pb_istream_t* istream,
                                          const pb_field_t* field,
                                          void** arg) {
    TimeSeries* ts = (TimeSeries*)*arg;
    TimeSeries::Label* l = ts->_labels[ts->_labelsPointer];

    size_t bytesLeft = istream->bytes_left;

    if (l->_maxValLen < istream->bytes_left) {
        Serial.println("label val is longer than allocated buffer");
        ts->errmsg = (char*)"Label val is longer than allocated buffer";
        return false;
    }

    if (!pb_read(istream, (unsigned char*)l->val, istream->bytes_left))
        return false;

    // We allocated enough space for the max length plus 1 for a terminator
    l->val[bytesLeft] = '\0';

    return true;
}

bool WriteRequest::callback_encode_timeseries(pb_ostream_t* ostream,
                                              const pb_field_t* field,
                                              void* const* arg) {
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

bool WriteRequest::callback_encode_labels(pb_ostream_t* ostream,
                                          const pb_field_t* field,
                                          void* const* arg) {
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

bool WriteRequest::callback_encode_string(pb_ostream_t* ostream,
                                          const pb_field_t* field,
                                          void* const* arg) {
    char* s = (char*)*arg;

    if (!pb_encode_tag_for_field(ostream, field)) {
        return false;
    }
    if (!pb_encode_string(ostream, (const uint8_t*)s, strlen(s))) {
        return false;
    }
    return true;
}

bool WriteRequest::callback_encode_samples(pb_ostream_t* ostream,
                                           const pb_field_t* field,
                                           void* const* arg) {
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
