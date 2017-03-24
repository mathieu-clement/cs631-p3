#include "analysis.h"

#include <stdio.h>

void print_analysis(struct analysis a)
{
   printf("# of instructions executed:          %d\n"
          "# of data processing instructions:   %d\n"
          "# of branches:                       %d\n"
          "# of branches taken:                 %d\n"
          "# of branches not taken:             %d\n",
          a.instructions,
          a.dp_instructions,
          a.branches,
          a.branches_taken,
          a.branches_not_taken
         );
}
