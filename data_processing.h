#ifndef DATA_PROCESSING_H
#define DATA_PROCESSING_H

#include <stdbool.h>

#include "state.h"

// Data processing instruction
// DDCA, page 330
// Due to endianness, wasn't able to use the bitfield directly
struct dp_instr {
    unsigned int cond  :  4  ;
    unsigned int op    :  2  ;
    unsigned int i     :  1  ;
    unsigned int cmd   :  4  ;
    unsigned int s     :  1  ;
    unsigned int rn    :  4  ;
    unsigned int rd    :  4  ;
    unsigned int src2  : 12  ;
};

struct dp_instr decode_dp_instr (unsigned int raw);

void armemu_one_dp(struct state* state, struct dp_instr* inst);

#endif
