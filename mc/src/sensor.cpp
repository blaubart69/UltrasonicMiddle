#include <functional>
#include <Arduino.h>

#include "sensor.h"
#include "stats.h"

#include "RingBuffer.h"


static bool checksum_valid_2(unsigned char* data) // == char[4]
{
	const int checksum = ( data[0] + data[1] + data[2] ) & 0xFF;
	return checksum == data[3];
}

static bool read_millimeter_from_serial(Stream* serial, STATS* stats, unsigned int* millimeter) {
  unsigned char data[4];
  data[0] = 0xFF;

  for (;;) {
    int b = serial->read();
    if ( b == -1) {
      break;
    }    
    else if ( b != 0xFF ) {
      stats->notFF += 1;
    }
    else if ( serial->readBytes(&(data[1]),3) != 3 ) {
      stats->ne += 1;
    }
    else if ( ! checksum_valid_2(data) ) {
      stats->data_error += 1;
    }
    else {
      stats->data_ok += 1;
		  *millimeter = ( data[1] << 8 ) + data[2];

      stats->moreData += serial->available();

      return true;
    }
  }
  return false;
}

bool          pair_has_value[2] = {false, false};

static ValuePair   pair;
static ValuePair   pair_summed;
static RingBuffer  ring_buf;
static unsigned long last_millis = 0;

void serialOnReceive(Stream* serial, const int sensor, const int avg_range, STATS* stats, std::function<void(ValuePair*)> onPairReady) {
  
  const unsigned long curr_millis = millis();
  if ( last_millis != 0) {
    stats->gap_ms += (curr_millis - last_millis);
  }
  last_millis = curr_millis;

  unsigned int millimeter;

  if (!read_millimeter_from_serial(serial, stats, &millimeter)) {
    // 
  }
  else {
    if ( pair_has_value[sensor] ) {
      stats->overwrite += 1;
    }

    pair.val[sensor]       = millimeter;
    pair_has_value[sensor] = true;

    if ( pair_has_value[1-sensor] ) {

      ring_buf.push(pair);
      
      const int summed_pairs = ring_buf.sum_last_values(avg_range, &pair_summed);
      if ( summed_pairs < (avg_range/2) ) {
        pair_summed.reset();
      }
      else {
        pair_summed.val[0] /= avg_range;
        pair_summed.val[1] /= avg_range;
      }
      
      onPairReady(&pair_summed);

      pair_has_value[0] = false;
      pair_has_value[1] = false;
    }
  }
}
