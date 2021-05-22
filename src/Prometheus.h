
#ifndef prometheus_h
#define prometheus_h

#include <Arduino.h>
#include "WriteRequest.h"

#if defined(ESP32)

#include "clients/ESP32Client.h"
typedef ESP32Client Prometheus;

#else 

#error "Unknown board type, PrometheusArduino does not support this board."

#endif


#endif