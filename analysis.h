#ifndef ANALYSIS_H
#define ANALYSIS_H

struct analysis {
    unsigned int instructions;
    unsigned int dp_instructions;
    unsigned int branches;
    unsigned int branches_taken;
    unsigned int branches_not_taken;
};

void print_analysis(struct analysis a);

#endif
