#pragma once

#include <functional>
#include <Stream.h>

#include "stats.h"
#include "RingBuffer.h"



void serialOnReceive(Stream* serial, const int sensor, const int avg_range, STATS* stats, std::function<void(ValuePair*)> onPairReady);