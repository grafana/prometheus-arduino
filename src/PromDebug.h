#ifndef promdebug_h
#define promdebug_h


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

#define PRINT_HEAP() \
    { \
        if (_debug) { \
            _debug->print("Free Heap: "); \
            _debug->println(freeMemory()); \
        } \
    } \

#ifndef FREEMEM
#define FREEMEM
#if defined(ESP32) || defined(ESP8266)

#include <Esp.h>
inline int freeMemory() {
    return ESP.getFreeHeap();
}

#else

#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else  // __ARM__
extern char* __brkval;
#endif  // __arm__

inline int freeMemory() {
    char top;
#ifdef __arm__
    return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
    return &top - __brkval;
#else  // __arm__
    return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif  // __arm__
}

#endif // Type

#endif // FREEMEM

#endif // debug_h
