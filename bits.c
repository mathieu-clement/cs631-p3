#include "bits.h"

unsigned int select_bits(unsigned int src, int high, int low)
{
    unsigned int mask = 0xFFFFFFFF;
    int nb_ones = 1 + high - low;

    mask >>= 32 - nb_ones;

    int shift = high - nb_ones + 1;
    mask <<= shift;
    return (mask & src) >> shift;
}
