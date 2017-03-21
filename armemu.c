#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 1
#include "debug_utils.h"

#include "bits.h"
#include "branch.h"
#include "data_processing.h"
#include "memory.h"
#include "state.h"

// Assembly function signature
typedef unsigned int (*func)(unsigned int, unsigned int, unsigned int, unsigned int);

unsigned int add_function (unsigned int a, unsigned int b, unsigned int c, unsigned int d);
unsigned int sum_array(unsigned int a, unsigned int b, unsigned int c, unsigned int d);

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

    s->cpsr = 0;

    s->regs[SP] = (unsigned int) &(s->stack[STACK_SIZE+1]);
    s->regs[PC] = (unsigned int) f;
    s->regs[LR] = 0;

    debug("SP = 0x%02x", s->regs[SP]);
    debug("PC = 0x%02x", s->regs[PC]);
    debug("LR = 0x%02x", s->regs[LR]);
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

#define COND_EQ 0
#define COND_NE 1
#define COND_LS 9
#define COND_GE 10
#define COND_LT 11
#define COND_LE 13
#define COND_AL 14

bool condition_is_true (struct state* s, unsigned int cond)
{
    // bit numbers from https://www.heyrick.co.uk/armwiki/The_Status_register
    unsigned int cpsr = s->cpsr;
    unsigned int n = select_bits(cpsr, 31, 31);
    unsigned int z = select_bits(cpsr, 30, 30);
    unsigned int c = select_bits(cpsr, 29, 29);
    unsigned int v = select_bits(cpsr, 28, 28);
    // unsigned int q = select_bits(cpsr, 27, 27);

    switch (cond) {
        case COND_EQ:
            return z == 0;
        case COND_NE:
            return z == 1;
        case COND_LS:
            return c == 0 || z == 1;
        case COND_GE:
            return n == v;
        case COND_LT:
            return n != v;
        case COND_LE:
            return z == 1 || n != v;
        case COND_AL:
            return true;
        default:
            fprintf(stderr, "Unknown condition 0x%02x\n", cond);
            exit(EXIT_FAILURE);
    }
}

void armemu_one (struct state* s)
{
    unsigned int* pc_addr = (unsigned int*) s->regs[PC];
    debug("PC is at address 0x%02x", s->regs[PC]);
    struct dp_instr dp_instr = decode_dp_instr(*pc_addr);
    //print_instr(&dp_instr);
    
    if (condition_is_true(s, dp_instr.cond)) {
        debug("Condition %d is true", dp_instr.cond);

        switch (dp_instr.op) {
            case 0x00: // Data processing
                armemu_one_dp(s, &dp_instr);
                break;
            case 0x01: // Data transfer (LDR, STR)
                {
                    struct load_store_instr instr = decode_load_store_instr(*pc_addr);
                    armemu_one_load_store(s, &instr);
                    break;
                }
            case 0x02: // Branch and link or STM/LDM
                { 
                    unsigned int code = select_bits(*pc_addr, 27, 25);
                    if (code == 0x5) { // Branch
                        struct branch_link_instr bl_instr = decode_branch_link_instr(*pc_addr);
                        armemu_one_branch(s, &bl_instr);
                    } else if (code == 0x4) { // STM/LDM
                        struct load_store_multiple_instr lsm_instr = decode_load_store_multiple_instr(*pc_addr);
                        armemu_one_load_store_multiple(s, &lsm_instr);
                        break;
                    } else {
                        fprintf(stderr, "Unknown instruction %02x \n", *pc_addr);
                        exit(EXIT_FAILURE);
                    }
                    break;
                }
            default:
                fprintf(stderr, "Unknown instruction type (op) %02x.\n", dp_instr.op);
                exit(EXIT_FAILURE);
        } // end switch
    } else {
        debug("Condition %d is false", dp_instr.cond);
    }

    // Update PC
    if (dp_instr.rn != PC) {
        s->regs[PC] = s->regs[PC] + 4;
    }
}

void armemu(struct state* s)
{
    int i = 0;
    while (s->regs[PC] != 0) {
        debug("Instruction #%d", ++i); 
        armemu_one(s); 
    }
    debug("Reached function end. %d instructions executed.", i);
}

int main (int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s FUNCTION\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct state state = { };

    if (strcmp(argv[1], "add_function") == 0) {
        init_state(&state, add_function, 10, 11, 12, 13);
    } else if (strcmp(argv[1], "sum_array") == 0) {
        int array[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 } ;
        init_state(&state, sum_array, (unsigned int) array, 9, 0, 0);
    } else {
        fprintf(stderr, "Unknown function %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    armemu(&state);
    printf("r = %d\n", state.regs[0]);
}

