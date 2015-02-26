// Implement your solutions in this file
#include "hamming.h"

const uint64_t FLAG = 0x01;

unsigned int hamming(uint64_t x, uint64_t y)
{
    uint64_t record = x ^ y;

    unsigned int count = 0;
    while(record != 0)
    {
        count += (record & FLAG);
        record >>= 1;
    }

    return count;
}

