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

    deserializeJson(*_json, *stream);
    

    DEBUG_PRINT("End deserialization: ");
    PRINT_HEAP();
    return 0;
}

void ReadRequest::urlEncode(char* str, uint16_t len, char* output){

}
