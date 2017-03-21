#ifndef STATE_H
#define STATE_H

#define NUM_REGS 16
#define SP 13
#define LR 14
#define PC 15
#define STACK_SIZE 4096

struct state {
    unsigned int regs[NUM_REGS];
    unsigned int cpsr;
    unsigned int stack[STACK_SIZE];
};

// Assembly function signature
typedef unsigned int (*func)(unsigned int, unsigned int, unsigned int, unsigned int);

void init_state(struct state* s, 
                func f, 
                unsigned int r0, 
                unsigned int r1,
                unsigned int r2,
                unsigned int r3);

#endif
