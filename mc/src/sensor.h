#pragma once

#include <functional>
#include <Stream.h>

#include "stats.h"
#include "RingBuffer.h"

struct SensorSettings {
    unsigned int avg_values = 10;
    unsigned int threshold_cm = 2;
};

void serialOnReceive(Stream* serial, const int sensor, STATS* stats, std::function<void(ValuePair*)> onPairReady);