#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>

#include "debug_utils.h"

#include "bits.h"
#include "branch.h"
#include "conditions.h"
#include "data_processing.h"
#include "memory.h"
#include "state.h"

#define ITERS 10000

#define OP_DATA_PROCESSING 0
#define OP_SINGLE_DATA_TRANSFER 1
#define OP_BL_OR_MULTIPLE_DATA_TRANSFER 2
#define CODE_BRANCH 5
#define CODE_STM_LDM 4

#define FUNC_DEF(name) unsigned int name (unsigned int a, unsigned int b, unsigned int c, unsigned int d)

FUNC_DEF(add_function);
FUNC_DEF(sum_array);
FUNC_DEF(fib_iter);
FUNC_DEF(fib_rec);
FUNC_DEF(find_max);
FUNC_DEF(find_str);

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
                if (dp_instr.cmd == DP_CMD_BX) {
                    s->analysis.branches++;
                    if (cond_true) {
                        s->analysis.branches_taken++;
                    } else {
                        s->analysis.branches_not_taken++;
                    }
                } else {
                    s->analysis.dp_instructions++;
                }

                if (cond_true) {
                    armemu_one_dp(s, &dp_instr);
                }
                break;
            }
        case OP_SINGLE_DATA_TRANSFER:
            {
                s->analysis.memory_instructions++;
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
                    s->analysis.memory_instructions++;
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

void armemu (struct state* s)
{
    while (s->regs[PC] != 0) {
        debug("Instruction #%d", ++s->analysis.instructions); 
        armemu_one(s); 
    }
    debug("Reached function end. %d instructions executed.", s->analysis.instructions);
}

void invoke (
        func func, 
        unsigned int a, 
        unsigned int b, 
        unsigned int c,
        unsigned int d
        )
{
    struct state state;

    printf("#########################\n"
           "# NATIVE IMPLEMENTATION #\n"
           "#########################\n\n");

    int native_result = func(a, b, c, d);
    printf("native r = %d\n\n", native_result);

    // Measure performance of native function
    struct tms native_tms_before;
    struct tms native_tms_after;

    times(&native_tms_before);

    for (int i = 0 ; i < ITERS; ++i) {
        func(a, b, c, d);
    }

    times(&native_tms_after);

    clock_t native_utime_n_iters = native_tms_after.tms_utime - native_tms_before.tms_utime;
    printf("# clock ticks for %d iterations of native implementation: %ld\n", ITERS, native_utime_n_iters);
    double native_utime_avg = native_utime_n_iters / (double) ITERS;
    printf("Average per iteration: %lf\n", native_utime_avg);

    printf("\n");

    printf("###########################\n"
           "# EMULATED IMPLEMENTATION #\n"
           "###########################\n\n");

    // Measure performance of emulated function
    struct tms emulated_tms_before;
    struct tms emulated_tms_after;

    times(&emulated_tms_before);

    for (int i = 0 ; i < ITERS; ++i) {
        init_state(&state, func, a, b, c, d);
        armemu(&state);
    }

    times(&emulated_tms_after);

    printf("emulated r = %d\n\n", (int) state.regs[0]);

    clock_t emulated_utime_n_iters = emulated_tms_after.tms_utime - emulated_tms_before.tms_utime;
    printf("# clock ticks for %d iterations of emulated implementation: %ld\n", ITERS, emulated_utime_n_iters);
    double emulated_utime_avg = emulated_utime_n_iters / (double) ITERS;
    printf("Average per iteration: %lf\n", emulated_utime_avg);
    printf("\n");
    if (native_utime_n_iters > 0) {
        printf("Emulation is %ld times slower than native implementation.\n", emulated_utime_n_iters / native_utime_n_iters);
        printf("\n");
    }

    print_analysis(state.analysis);
    printf("r = %d\n", state.regs[0]);
}

void check_num_args (int expected, int actual)
{
    actual -= 2; // 0 is program name, 1 is function name
    if (actual != expected) {
        fprintf(stderr, "missing arguments. %d expected, found %d, including program name.\n",
                expected, actual);
        exit(EXIT_FAILURE);
    }
}

bool is_function_invocation(char* func_name, char* argv[])
{
    return strcmp(argv[1], func_name) == 0;
}

int main (int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s FUNCTION\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (is_function_invocation("add_function", argv)) {
        invoke(add_function, 10, 11, 12, 13);
    } else if (is_function_invocation("sum_array", argv)) {
        int array[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 } ;
        invoke(sum_array, (unsigned int) array, 9, 0, 0);
    } else if (is_function_invocation("find_max", argv)) {
        int array[] = { 1, 2, 3, 4, 32, 5, 6, 7, 8, 9 } ;
        invoke(find_max, (unsigned int) array, 10, 0, 0);
    } else if (is_function_invocation("find_str", argv)) {
        check_num_args(2, argc);
        invoke(find_str, (unsigned int) argv[2], (unsigned int) argv[3], 0, 0);
    } else if (is_function_invocation("fib_iter", argv)) {
        check_num_args(1, argc);
        invoke(fib_iter, atoi(argv[2]), 0, 0, 0);
    } else if (is_function_invocation("fib_rec", argv)) {
        check_num_args(1, argc);
        invoke(fib_rec, atoi(argv[2]), 0, 0, 0);
    } else {
        fprintf(stderr, "Unknown function %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }
}

