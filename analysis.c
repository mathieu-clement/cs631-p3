#include "analysis.h"
#include "armemu.h"
#include "func.h"
#include "state.h"

#include <stdio.h>
#include <sys/times.h>

void print_analysis (struct analysis a)
{
    printf("########################\n"
           "#  ANALYSIS / REPORT   #\n"
           "########################\n\n");

    printf("# of instructions executed:          %d\n"
           "# of data processing instructions:   %d\n"
           "# of memory instructions:            %d\n"
           "# of branches:                       %d\n"
           "# of branches taken:                 %d\n"
           "# of branches not taken:             %d\n",
           a.instructions,
           a.dp_instructions,
           a.memory_instructions,
           a.branches,
           a.branches_taken,
           a.branches_not_taken
          );

    printf("\n");

    printf("+----------+---------+----------+\n");
    printf("| Register | # reads | # writes |\n");
    printf("|----------+---------+----------|\n");
    for (int reg = 0 ; reg < NUM_REGS; ++reg) {
        printf("| r%-7d | %7d | %8d |\n", 
               reg, a.register_reads[reg], a.register_writes[reg]);
    }
    printf("+----------+---------+----------+\n");
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
}

