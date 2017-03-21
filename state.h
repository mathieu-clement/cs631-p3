#ifndef STATE_H
#define STATE_H

#define NUM_REGS 16
#define SP 13
#define LR 14
#define PC 15
#define STACK_SIZE 4096

struct cpsr {
    unsigned int n  : 1 ;
    unsigned int z  : 1 ;
    unsigned int c  : 1 ;
    unsigned int v  : 1 ;
    unsigned int q  : 1 ;
    unsigned int    : 1 ;
    unsigned int j  : 1 ;
    unsigned int    : 1 ;
    unsigned int ge : 4 ;
    unsigned int    : 1 ;
    unsigned int e  : 1 ;
    unsigned int a  : 1 ;
    unsigned int i  : 1 ;
    unsigned int f  : 1 ;
    unsigned int t  : 1 ;
    unsigned int m  : 5 ;
};

struct state {
    unsigned int regs[NUM_REGS];
    struct cpsr cpsr;
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
