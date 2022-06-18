#include <string.h>
#include "RingBuffer.h"

void RingBuffer::push(const ValuePair& pair) 
{
    if ( _write_idx == BUFFER_SIZE ) {
        _write_idx = 0;
    }

    _buf[_write_idx].val[0] = pair.val[0];
    _buf[_write_idx].val[1] = pair.val[1];

    ++_write_idx;
    ++_count_inserted;
}

RingBuffer::RingBuffer() {
    memset(_buf, 0, sizeof(_buf));
}

int RingBuffer::sum_last_values(int count, ValuePair* sumPair) const
{
    int pairs_summed = 0;
    sumPair->reset();

    if (count > BUFFER_SIZE) {
        count = BUFFER_SIZE;
    }

    if ( _count_inserted < count ) {
        count = _count_inserted;
    }

    int read_idx = _write_idx;
    for (int i=count; i > 0; --i) {

        read_idx -= 1;

        if ( read_idx < 0 ) {
            read_idx = BUFFER_SIZE-1;
        }

        if (   _buf[read_idx].val[0] != 0 
            && _buf[read_idx].val[1] != 0 ) 
        {
            sumPair->val[0] += _buf[read_idx].val[0];
            sumPair->val[1] += _buf[read_idx].val[1];
            pairs_summed++;
        }
    }

    return pairs_summed;
}