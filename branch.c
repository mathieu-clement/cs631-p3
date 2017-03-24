#include "branch.h"

#include "bits.h"
#include "data_processing.h"
#include "debug_utils.h"
#include "state.h"

struct branch_link_instr decode_branch_link_instr (unsigned int raw)
{
    return (struct branch_link_instr) {
        .link   = select_bits(raw, 24, 24),
        .offset = select_bits(raw, 23, 0)
    };
}

void branch_and_exchange(struct state* state, struct dp_instr* inst)
{
    debug("Branch and exchange", NULL);
    unsigned int rn = select_bits(inst->src2, 3, 0);
    unsigned int rn_val = state->regs[rn];
    debug("Current value of PC: 0x%02x", state->regs[PC]);
    debug("Current value of LR: 0x%02x", state->regs[LR]);
    state->regs[PC] = rn_val;
    debug("Update PC to 0x%02x", rn_val);
}

void armemu_one_branch(struct state* state, struct branch_link_instr* instr)
{
    debug("Branch (and link)", NULL);
    if (instr->link) {
        debug("Is link instruction, LR = PC (0x%02x) + 4 = 0x%02x", state->regs[PC], state->regs[PC]+4); 
        state->regs[LR] = state->regs[PC] + 4;
    } else {
        debug("NOT a link instruction, LR not updated", NULL);
    }

    unsigned int raw_offset = instr->offset;
    raw_offset <<= 2;
    
    // Sign extend offset
    // Because integers are 32 bit, and we shifted a 24-bit number by 2
    // there are 6 bits that must be set to 1. (32 - 24 - 2 = 6)
    if (select_bits(raw_offset, 25, 25) == 1) {
        raw_offset |= 0xFC000000;
    }

    signed int signed_offset = (signed int) raw_offset;

    state->regs[PC] = state->regs[PC] + 8 + signed_offset; // 8 simulates prefetch
    debug("Branch to 0x%02x", state->regs[PC]);
}
