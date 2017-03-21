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

