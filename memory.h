#ifndef MEMORY_H
#define MEMORY_H

#include "state.h"

struct load_store_instr {
    unsigned int cond   :  4 ;
    unsigned int op     :  2 ;
    unsigned int i      :  1 ;
    unsigned int p      :  1 ;
    unsigned int u      :  1 ;
    unsigned int b      :  1 ;
    unsigned int w      :  1 ;
    unsigned int l      :  1 ;
    unsigned int rn     :  4 ;
    unsigned int rd     :  4 ;
    unsigned int offset : 12 ;
};

struct load_store_instr decode_load_store_instr (unsigned int raw);
void armemu_one_load_store (struct state* s, struct load_store_instr* instr);

struct load_store_multiple_instr {
    unsigned int cond :  4 ;
    unsigned int      :  3 ;
    unsigned int p    :  1 ;
    unsigned int u    :  1 ;
    unsigned int s    :  1 ;
    unsigned int w    :  1 ;
    unsigned int l    :  1 ;
    unsigned int rn   :  4 ;
    unsigned int regs : 16 ;
};

struct load_store_multiple_instr decode_load_store_multiple_instr (unsigned int raw);
void armemu_one_load_store_multiple (struct state* s, struct load_store_multiple_instr* instr);

#endif
