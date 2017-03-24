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

#define OP_DATA_PROCESSING 0
#define OP_SINGLE_DATA_TRANSFER 1
#define OP_BL_OR_MULTIPLE_DATA_TRANSFER 2
#define CODE_BRANCH 5
#define CODE_STM_LDM 4

unsigned int add_function (unsigned int a, unsigned int b, unsigned int c, unsigned int d);
unsigned int sum_array(unsigned int a, unsigned int b, unsigned int c, unsigned int d);
unsigned int fib_iter(unsigned int a, unsigned int b, unsigned int c, unsigned int d);
unsigned int fib_rec(unsigned int a, unsigned int b, unsigned int c, unsigned int d);
unsigned int find_max(unsigned int a, unsigned int b, unsigned int c, unsigned int d);
unsigned int find_str(unsigned int a, unsigned int b, unsigned int c, unsigned int d);

void armemu_one (struct state* s)
{
    unsigned int* pc_addr = (unsigned int*) s->regs[PC];
    debug("PC is at address 0x%02x", s->regs[PC]);

    // All instructions are first decoded as data processing instructions
    // due to the presence of the condition and op code fields.
    struct dp_instr dp_instr = decode_dp_instr(*pc_addr);
    
    unsigned int code = select_bits(*pc_addr, 27, 25);

    const char* cond_str = condition_to_string(dp_instr.cond);
    bool cond_true = condition_is_true(s, dp_instr.cond);

    bool is_branch = false;
    
    if (dp_instr.cond != COND_AL && cond_true) debug("Condition %s is true", cond_str);

    switch (dp_instr.op) {
        case OP_DATA_PROCESSING:
            {
                s->analysis.dp_instructions++;
                if (cond_true) armemu_one_dp(s, &dp_instr);
                break;
            }
        case OP_SINGLE_DATA_TRANSFER:
            {
                if (cond_true) {
                    struct load_store_instr instr = decode_load_store_instr(*pc_addr);
                    armemu_one_load_store(s, &instr);
                }
                break;
            }
        case OP_BL_OR_MULTIPLE_DATA_TRANSFER:
            { 
                if (code == CODE_BRANCH) { // Branch
                    s->analysis.branches++;
                    if (cond_true) {
                        is_branch = true;
                        s->analysis.branches_taken++;
                        struct branch_link_instr bl_instr = decode_branch_link_instr(*pc_addr);
                        armemu_one_branch(s, &bl_instr);
                    } else {
                        s->analysis.branches_not_taken++;
                    }
                } else if (code == CODE_STM_LDM) { // STM/LDM
                    if (cond_true) {
                        struct load_store_multiple_instr lsm_instr = decode_load_store_multiple_instr(*pc_addr);
                        armemu_one_load_store_multiple(s, &lsm_instr);
                    }
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

    if (!cond_true) debug("Condition %s is false", cond_str);

    // Update PC
    if (!is_branch && (!cond_true || dp_instr.rn != PC)) {
        s->regs[PC] = s->regs[PC] + 4;
    } else {
        debug("PC not incremented", NULL);
    }
}

void armemu(struct state* s)
{
    while (s->regs[PC] != 0) {
        debug("Instruction #%d", ++s->analysis.instructions); 
        armemu_one(s); 
    }
    debug("Reached function end. %d instructions executed.", s->analysis.instructions);
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
    } else if (strcmp(argv[1], "find_max") == 0) {
        int array[] = { 1, 2, 3, 4, 32, 5, 6, 7, 8, 9 } ;
        init_state(&state, find_max, (unsigned int) array, 10, 0, 0);
    } else if (strcmp(argv[1], "find_str") == 0) {
        if (argc < 4) {
            fprintf(stderr, "missing arguments\n");
            exit(EXIT_FAILURE);
        }
        init_state(&state, find_str, (unsigned int) argv[2], (unsigned int) argv[3], 0, 0);
    } else if (strcmp(argv[1], "fib_iter") == 0) {
        if (argc < 3) {
            fprintf(stderr, "missing argument \n");
            exit(EXIT_FAILURE);
        }
        init_state(&state, fib_iter, atoi(argv[2]), 0, 0, 0);
    } else if (strcmp(argv[1], "fib_rec") == 0) {
        if (argc < 3) {
            fprintf(stderr, "missing argument \n");
            exit(EXIT_FAILURE);
        }
        init_state(&state, fib_rec, atoi(argv[2]), 0, 0, 0);
    } else {
        fprintf(stderr, "Unknown function %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    armemu(&state);

    print_analysis(state.analysis);
    printf("r = %d\n", state.regs[0]);
}

