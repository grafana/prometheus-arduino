#ifndef debug_h
#define debug_h


#define DEBUG_PRINT(...)               \
    {                                  \
        if (_debug) {                  \
            _debug->print(__VA_ARGS__); \
        }                              \
    }
#define DEBUG_PRINTLN(...)                \
    {                                     \
        if (_debug) {                     \
            _debug->println(__VA_ARGS__);  \
        }                                 \
    }


#endif