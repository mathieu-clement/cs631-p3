#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

unsigned int add (unsigned int a, unsigned int b, unsigned int c, unsigned int d);

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

unsigned int select_bits(unsigned int src, int high, int low)
{
    unsigned int mask = 0;
    int nb_ones = 1 + high - low;

    for (int i = 0 ; i < nb_ones ; ++i) {
        mask <<= 1;
        mask |= 1;
    }

    int shift = high - nb_ones + 1;
    
    mask <<= shift;
    return (mask & src) >> shift;
}

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

struct dp_instr decode_dp_instr (unsigned int raw)
{
    return (struct dp_instr) {
        .cond   =     select_bits(raw, 31, 28) ,
        .op     =     select_bits(raw, 27, 26) ,
        .i      =     select_bits(raw, 25, 25) ,
        .cmd    =     select_bits(raw, 24, 21) ,
        .s      =     select_bits(raw, 20, 20) ,
        .rn     =     select_bits(raw, 19, 16) ,
        .rd     =     select_bits(raw, 15, 12) ,
        .src2   =     select_bits(raw, 11, 0) 
    };
}

typedef unsigned int (*func)(unsigned int, unsigned int, unsigned int, unsigned int);

void init_state(struct state* s, func f, 
                unsigned int r0, 
                unsigned int r1,
                unsigned int r2,
                unsigned int r3)
{
    printf("%d %d %d %d\n", r0, r1, r2, r3);

    for (int i = 0 ; i < NUM_REGS ; ++i) {
        s->regs[i] = 0;
    }

    s->cpsr = 0;

    s->regs[SP] = (unsigned int) &(s->stack[STACK_SIZE+1]);
    s->regs[PC] = (unsigned int) f;
    s->regs[LR] = 0;
}

void print_instr(struct dp_instr* i)
{
    printf("cond: %u\n"
           "  op: %u\n"
           "   I: %u\n"
           " cmd: %u\n"
           "   S: %u\n"
           "  Rn: %u\n"
           "  Rd: %u\n"
           "Src2: %u\n",
           i->cond,
           i->op,
           i->i,
           i->cmd,
           i->s,
           i->rn,
           i->rd,
           i->src2
           );
}

void armemu_one(struct state* s)
{
    unsigned int* pc_addr = (unsigned int*) s->regs[PC];
    struct dp_instr dp_instr = decode_dp_instr(*pc_addr);
    print_instr(&dp_instr);
    if (dp_instr.rn != PC) {
        s->regs[PC] = s->regs[PC] + 4;
    }
}

void armemu(struct state* s)
{
    int i = 0;
//    while(s->regs[PC] != 0) {
    for ( ; i < 4 ; ++i) {
        printf("%d:\n", i); armemu_one(s); 
    }
}


int main (int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s FUNCTION\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct state state = { };

    init_state(&state, add, 1, 2, 3, 4);
    armemu(&state);
    // printf("r = %d\n", r);
}
