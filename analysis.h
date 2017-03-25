#ifndef ANALYSIS_H
#define ANALYSIS_H

#ifndef NUM_REGS
#define NUM_REGS 16
#endif

struct analysis {
    unsigned int instructions;
    unsigned int dp_instructions;
    unsigned int memory_instructions;
    unsigned int branches;
    unsigned int branches_taken;
    unsigned int branches_not_taken;
    unsigned int register_writes[NUM_REGS];
    unsigned int register_reads[NUM_REGS];
};

void print_analysis (struct analysis a);

#endif
