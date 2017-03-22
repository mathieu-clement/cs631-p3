#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "debug_utils.h"

#include "bits.h"
#include "branch.h"
#include "conditions.h"
#include "data_processing.h"
#include "memory.h"
#include "state.h"

unsigned int add_function (unsigned int a, unsigned int b, unsigned int c, unsigned int d);
unsigned int sum_array(unsigned int a, unsigned int b, unsigned int c, unsigned int d);
unsigned int fib_iter(unsigned int a, unsigned int b, unsigned int c, unsigned int d);

void armemu_one (struct state* s)
{
    unsigned int* pc_addr = (unsigned int*) s->regs[PC];
    debug("PC is at address 0x%02x", s->regs[PC]);
    struct dp_instr dp_instr = decode_dp_instr(*pc_addr);
    
    const char* cond_str = condition_to_string(dp_instr.cond);
    bool cond_true = condition_is_true(s, dp_instr.cond);
    
    if (cond_true) {
        if (dp_instr.cond != 14) debug("Condition %s is true", cond_str);
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
        debug("Condition %s is false", cond_str);
    } // end if condition

    // Update PC
    if (!cond_true || dp_instr.rn != PC) {
        s->regs[PC] = s->regs[PC] + 4;
    } else {
        debug("PC not incremented", NULL);
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
    } else if (strcmp(argv[1], "fib_iter") == 0) {
        init_state(&state, fib_iter, 3, 0, 0, 0);
    } else {
        fprintf(stderr, "Unknown function %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
    armemu(&state);
    printf("r = %d\n", state.regs[0]);
}

