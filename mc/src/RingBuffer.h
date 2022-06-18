#pragma once

struct ValuePair {
    unsigned int  val[2];

    ValuePair() {
        reset();
    }

    void reset() {
        val[0] = 0;
        val[1] = 0;
    };

};

#define BUFFER_SIZE 128

class RingBuffer
{
    private:

        ValuePair       _buf[BUFFER_SIZE]; 
        unsigned long   _count_inserted = 0;
        int             _write_idx = 0;

    public:
        RingBuffer();
        void push(const ValuePair& pair);
        int  sum_last_values(int count, ValuePair* sumPair) const;


};