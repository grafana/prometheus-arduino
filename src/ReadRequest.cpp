#include "ReadRequest.h"

ReadRequest::ReadRequest(uint32_t numSeries, uint32_t bufferSize)
    : _seriesCount(numSeries), _bufferSize(bufferSize) {
    _series = new TimeSeries*[numSeries];
    _buffer = new uint8_t[bufferSize];
    _json = new DynamicJsonDocument(bufferSize);
}

ReadRequest::~ReadRequest() {
    delete[] _series;
    delete[] _buffer;
    delete _json;
}

void ReadRequest::setDebug(Stream& stream) {
    _debug = &stream;
}

bool ReadRequest::addTimeSeries(TimeSeries& series) {
    errmsg = nullptr;
    if (_seriesPointer >= _seriesCount) {
        errmsg = (char*)"cannot add series, max number of series have already been added.";
        return false;
    }

    _series[_seriesPointer] = &series;
    _seriesPointer++;
    return true;
}

uint32_t ReadRequest::getBufferSize() {
    return _bufferSize;
}

int16_t ReadRequest::fromHttpStream(Stream* stream) {
    DEBUG_PRINT("Begin deserialization: ");
    PRINT_HEAP();

    DeserializationError err = deserializeJson(*_json, *stream);
    if (err != DeserializationError::Ok) {
        DEBUG_PRINT("Failed to deserialize json: ");
        DEBUG_PRINTLN(err.c_str());
        errmsg = (char*)"failed to deserialize json, enable debug logging for more info.";
        return -1;
    }

    // DEBUG_PRINT("Deserialized json: ");
    // DEBUG_PRINTLN(_json->as<String>());

    DynamicJsonDocument doc = *_json;

    const char* resultTupe = doc["data"]["resultType"].as<const char*>();
    JsonArray result = doc["data"]["result"].as<JsonArray>();

    if (strcmp(resultTupe, "vector") == 0) {
        // Each element in the result is a TimeSeries
        uint8_t tsPointer = 0;
        for (JsonVariant v : result) {
            JsonObject obj = v.as<JsonObject>();
            JsonObject metric = obj["metric"].as<JsonObject>();
            uint8_t labelPos = 0;
            for (JsonPair p : metric) {
                const char* key = p.key().c_str();
                const char* value = p.value().as<const char*>();
                if(strcmp(key, "__name__") == 0) {
                    _series[tsPointer]->setName((char*)value, strlen(value));
                    continue;
                }
                //TODO should handle the error if the label doesn't get set properly.
                _series[tsPointer]->setLabel(labelPos, (char*)key, strlen(key), (char*)value, strlen(value));
                labelPos++;
            }

            int64_t ts = obj["value"][0].as<int64_t>();
            double val = obj["value"][1].as<double>();
            _series[tsPointer]->resetSamples();
            _series[tsPointer]->addSample(ts, val);
            tsPointer++;
            // If there are more series than we allocated, overwrite the last one
            // TODO: should we error here instead?
            if (tsPointer >= _seriesCount) {
                tsPointer = _seriesCount - 1;
            }
        }
    }
    

    DEBUG_PRINT("End deserialization: ");
    PRINT_HEAP();
    return 0;
}
