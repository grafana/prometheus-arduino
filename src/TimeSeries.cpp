#include "TimeSeries.h"

TimeSeries::TimeSeries(uint16_t batchSize, const char* name, const char* labels) : _batchSize(batchSize) {
    _batch = new TimeSeries::Sample * [batchSize];

    //Pre-allocate the memory for each Sample
    for (int i = 0; i < batchSize; i++) {
        TimeSeries::Sample* s = new TimeSeries::Sample();
        _batch[i] = s;
    }

    // Count the number of labels
    uint8_t numLabelPairs = 0;
    uint16_t lc = 0;
    while (labels[lc] != '\0') {
        if (labels[lc] == '=') {
            numLabelPairs++;
        }
        lc++;
    }

    // Add one to store the __name__ label
    _numLabels = numLabelPairs + 1;
    _labels = new TimeSeries::Label * [_numLabels];

    uint8_t labelIdx = 0;

    // Make a copy of the name to get a char
    char nameLabel[strlen(name)];
    strcpy(nameLabel, name);

    // Set the metric name
    TimeSeries::Label* l = new TimeSeries::Label((char*)"__name__", nameLabel);
    _labels[labelIdx] = l;
    labelIdx++;

    // Make a copy so we can modify the labels
    char modLabels[strlen(labels)];
    strcpy(modLabels, labels);

    // Split on commas
    char delim[] = ",";
    char* ptr = strtok(modLabels, delim);

    while (ptr != NULL) {
        // Process label pair

        // key and val can't be bigger than the entire pair
        uint16_t maxlen = strlen(ptr);
        char key[maxlen];
        char val[maxlen];

        // Setup some position trackers
        uint8_t keyIdx = 0;
        uint8_t valIdx = 0;
        uint8_t equalPos = 0;

        for (int i = 0; i < maxlen; i++) {
            if (ptr[i] == '\\' || ptr[i] == '\"' || ptr[i] == '{' || ptr[i] == '}') {
                // Ignore any backslashes or " or {}
                continue;
            }
            if (ptr[i] == '=') {
                // If we hit an equals that's the end of the key
                equalPos = i;
                break;
            }
            key[keyIdx] = ptr[i];
            keyIdx++;
        }
        // Key is oversized, null out the rest of key
        for (int i = keyIdx; i < maxlen; i++) {
            key[i] = '\0';
        }

        // printf("%s\n", key);

        // Do basically the same for the value
        for (int i = equalPos + 1; i < maxlen; i++) {
            if (ptr[i] == '\\' || ptr[i] == '\"' || ptr[i] == '{' || ptr[i] == '}') {
                // Ignore any backslashes or " or {}
                continue;
            }
            val[valIdx] = ptr[i];
            valIdx++;
        }
        // Val is oversized, null out the rest of the val
        for (int i = valIdx; i < maxlen; i++) {
            val[i] = '\0';
        }

        // printf("%s\n", val);

        TimeSeries::Label* l = new TimeSeries::Label(key, val);
        _labels[labelIdx] = l;
        labelIdx++;

        // Look for next pair
        ptr = strtok(NULL, delim);
    }

}

TimeSeries::~TimeSeries() {
    for (int i = 0; i < _batchSize; i++) {
        delete _batch[i];
    }
    delete[] _batch;
}

bool TimeSeries::addSample(int64_t tsMillis, double val) {
    errmsg = nullptr;
    if (_batchPointer >= _batchSize) {
        errmsg = (char*)"batch full";
        return false;
    }

    _batch[_batchPointer]->tsMillis = tsMillis;
    _batch[_batchPointer]->val = val;
    _batchPointer++;

    return true;
}

void TimeSeries::resetSamples() {
    _batchPointer = 0;
}

TimeSeries::Sample::Sample() {

}

TimeSeries::Sample::~Sample() {

}

TimeSeries::Label::Label(char* k, char* v) {
    // Make sure to leave room for the null terminator
    size_t lenk = strlen(k);
    key = (char*)malloc(sizeof(char) * (lenk + 1));
    strcpy(key, k);
    size_t lenv = strlen(v);
    val = (char*)malloc(sizeof(char) * (lenv + 1));
    strcpy(val, v);
}

TimeSeries::Label::~Label() {
    free(key);
    free(val);
}

