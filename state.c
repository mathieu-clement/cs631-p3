#include "state.h"

#include "debug_utils.h"

void init_state(struct state* s, 
                func f, 
                unsigned int r0, 
                unsigned int r1,
                unsigned int r2,
                unsigned int r3)
{
    s->regs[0] = r0;
    s->regs[1] = r1;
    s->regs[2] = r2;
    s->regs[3] = r3;

    for (int i = 4 ; i < NUM_REGS ; ++i) {
        s->regs[i] = 0;
    }

    s->cpsr = (struct cpsr) { };

    s->regs[SP] = (unsigned int) &(s->stack[STACK_SIZE+1]);
    s->regs[PC] = (unsigned int) f;
    s->regs[LR] = 0;

    s->analysis = (struct analysis) { };

    debug("SP = 0x%02x", s->regs[SP]);
    debug("PC = 0x%02x", s->regs[PC]);
    debug("LR = 0x%02x", s->regs[LR]);
}
