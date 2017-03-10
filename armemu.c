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

void print_instr(unsigned int i)
{
    printf("cond: %u\n"
           "  op: %u\n"
           "   I: %u\n"
           " cmd: %u\n"
           "   S: %u\n"
           "  Rn: %u\n"
           "  Rd: %u\n"
           "Src2: %u\n",
           (0xF0000000 & i) >> 28, // cond
           (0x0C000000 & i) >> 26, // op
           (0x02000000 & i) >> 25, // I
           (0x01E00000 & i) >> 21, // cmd
           (0x00100000 & i) >> 20, // S
           (0x000F0000 & i) >> 16, // Rn
           (0x0000F000 & i) >> 12, // Rd
           (0x00000FFF & i)        // Src2
           );
}

void armemu_one(struct state* s)
{
    unsigned int* pc_addr = (unsigned int*) s->regs[PC];
    print_instr(*pc_addr);
//    if (instr->rn != PC) {
//        s->regs[PC] = s->regs[PC] + 4;
//    }
}

void armemu(struct state* s)
{
    int i = 0;
//    while(s->regs[PC] != 0) {
    for ( ; i < 4 ; ++i) {
        printf("%d:\n", i); armemu_one(s); i++;
        printf("%d:\n", i); armemu_one(s); i++;
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
