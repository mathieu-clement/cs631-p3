#ifndef BRANCH_H
#define BRANCH_H

#include "data_processing.h"
#include "state.h"

struct branch_link_instr {
    unsigned int link   :  1 ;
    unsigned int offset   : 24 ;
};

void branch_and_exchange (struct state* state, struct dp_instr* inst);
struct branch_link_instr decode_branch_link_instr (unsigned int raw);
void armemu_one_branch (struct state* state, struct branch_link_instr* instr);

#endif
