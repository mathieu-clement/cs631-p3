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

#endif
