#include "analysis.h"

#include <stdio.h>

void print_analysis (struct analysis a)
{
    printf("++++++++++++++++++++++++\n"
           "+  ANALYSIS / REPORT   +\n"
           "++++++++++++++++++++++++\n\n");

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

    printf("Register | # reads | # writes\n");
    for (int reg = 0 ; reg < NUM_REGS; ++reg) {
        printf("r%-7d | %7d | %8d\n", 
               reg, a.register_reads[reg], a.register_writes[reg]);
    }
}
