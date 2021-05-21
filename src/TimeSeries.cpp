#include "TimeSeries.h"

TimeSeries::TimeSeries(uint16_t batchSize, char* name, LabelSet labels[], uint8_t numLabelPairs) : _batchSize(batchSize) {
    _batch = new TimeSeries::Sample * [batchSize];

    //Pre-allocate the memory for each Sample
    for (int i = 0; i < batchSize; i++) {
        TimeSeries::Sample* s = new TimeSeries::Sample();
        _batch[i] = s;
    }

    // Add one to store the __name__ label
    _numLabels = numLabelPairs + 1;
    _labels = new TimeSeries::Label * [_numLabels];
    // Start at 1 because we are going to add the name label at position 0
    for (int i = 1; i < _numLabels; i++) {
        // Make sure to select from the incoming array starting at the 0 value by subtracting 1 from i
        TimeSeries::Label* l = new TimeSeries::Label(labels[i - 1].key, labels[i - 1].val);
        _labels[i] = l;
    }
    TimeSeries::Label* l = new TimeSeries::Label("__name__", name);
    _labels[0] = l;

}

TimeSeries::~TimeSeries() {
    for (int i = 0; i < _batchSize; i++) {
        delete _batch[i];
    }
    delete[] _batch;
}

bool TimeSeries::addSample(int64_t tsMillis, double val) {
    if (_batchPointer >= _batchSize) {
        errmsg = "batch full";
        return false;
    }

    _batch[_batchPointer]->tsMillis = tsMillis;
    _batch[_batchPointer]->val = val;
    _batchPointer++;
}

void TimeSeries::resetSamples() {
    _batchPointer = 0;
}

TimeSeries::Sample::Sample() {

}

TimeSeries::Sample::~Sample() {

}

TimeSeries::Label::Label(char* k, char* v) {
    key = (char*)malloc(sizeof(char) * strlen(k));
    strcpy(key, k);
    val = (char*)malloc(sizeof(char) * strlen(v));
    strcpy(val, v);
}

TimeSeries::Label::~Label() {
    free(key);
    free(val);
}

