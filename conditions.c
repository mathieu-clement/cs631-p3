#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "bits.h"
#include "debug_utils.h"
#include "state.h"

#include "conditions.h"

#define COND_EQ 0
#define COND_NE 1
#define COND_LS 9
#define COND_GE 10
#define COND_LT 11
#define COND_LE 13
#define COND_AL 14

char* condition_to_str(unsigned int cond)
{
    switch(cond) {
        case COND_EQ:
            return "EQ";
        case COND_NE:
            return "NE";
        case COND_LS:
            return "LS";
        case COND_GE:
            return "GE";
        case COND_LT:
            return "LT";
        case COND_LE:
            return "LE";
        case COND_AL:
            return "AL";
    }
    return "??";
}

bool condition_is_true (struct state* s, unsigned int cond)
{
    unsigned int n = s->cpsr.n;
    unsigned int z = s->cpsr.z; 
    unsigned int c = s->cpsr.c; 
    unsigned int v = s->cpsr.v; 

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

