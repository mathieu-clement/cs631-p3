#include <stdio.h>
#include <stdlib.h>

#include "bits.h"
#include "debug_utils.h"
#include "state.h"

#include "memory.h"

struct load_store_instr decode_load_store_instr (unsigned int raw)
{
    return (struct load_store_instr) {
        .cond   = select_bits(raw, 31, 28),
        .op     = select_bits(raw, 27, 26),
        .i      = select_bits(raw, 25, 25),
        .p      = select_bits(raw, 24, 24),
        .u      = select_bits(raw, 23, 23),
        .b      = select_bits(raw, 22, 22),
        .w      = select_bits(raw, 21, 21),
        .l      = select_bits(raw, 20, 20),
        .rn     = select_bits(raw, 19, 16),
        .rd     = select_bits(raw, 15, 12),
        .offset = select_bits(raw, 12, 0)
    };
}

void armemu_one_load_store (struct state* s, struct load_store_instr* instr)
{
    unsigned int* mem_addr = (unsigned int*) s->regs[instr->rn];
    unsigned int offset = 0;

    debug("rn: r%d, rd: r%d, raw offset value: 0x%02x", instr->rn, instr->rd, instr->offset);

    if (instr->p == 0) {
        fprintf(stderr, "Post offset for load / store instruction is not supported\n");
        exit(EXIT_FAILURE);
    }
    if (instr->w == 1) {
        fprintf(stderr, "Write-back for load / store instruction is not supported\n");
        exit(EXIT_FAILURE);
    }

    if (instr->i == 0) {
        // Immediate offset
        if (instr->u == 0) { // offset not Up => subtract offset from rd
            offset = - (instr->offset);
        } else {
            offset = + (instr->offset);
        } // end if offset up
        debug("Immediate offset: 0x%02x (up: %d)", instr->offset, instr->u);
    } else {
        // Register offset
        unsigned int rm = select_bits(instr->offset, 3, 0);
        unsigned int rm_val = s->regs[rm];
        unsigned int shift = select_bits(instr->offset, 11, 4);
        debug("Offset %d shifted by %d from register r%d", rm_val, shift, rm);
        if (shift != 0) {
            rm_val <<= shift;
        }
        offset = rm_val;
    } // end if immediate

    mem_addr += offset;

    unsigned int rd = instr->rd;

    if  (instr->l == 1) {
        // LOAD data from mem_addr to rd
        s->regs[rd] = *mem_addr;
        if (instr->b == 1) {
            s->regs[rd] &= 0x000000FF;
        } // end if byte
        debug("Load data (0x%02x) from address 0x%02x (incl. offset) to r%d",
              s->regs[rd], (unsigned int) mem_addr, rd);
    } else {
        // STORE data from rd to mem_addr
        *mem_addr = s->regs[rd];
        if (instr->b == 1) {
            *mem_addr &= 0x000000FF;
        } // end if byte
        debug("Store data (0x%02x) from r%d to address 0x%02x (incl. offset)",
              s->regs[rd], rd, (unsigned int) mem_addr);
    } // end if load
} // end armemu_one_load_store

struct load_store_multiple_instr decode_load_store_multiple_instr (unsigned int raw)
{
    return (struct load_store_multiple_instr) {
        .cond = select_bits(raw, 31, 28),
        .p    = select_bits(raw, 24, 24),
        .u    = select_bits(raw, 23, 23),
        .s    = select_bits(raw, 22, 22),
        .w    = select_bits(raw, 21, 21),
        .l    = select_bits(raw, 20, 20),
        .rn   = select_bits(raw, 19, 16),
        .regs = select_bits(raw, 15,  0)
    };
}

void armemu_one_load_store_multiple (struct state* s, struct load_store_multiple_instr* instr)
{
    if (instr->s == 1) {
        fprintf(stderr, "PSR & force user bit in multiple load / store instruction is not supported\n");
        exit(EXIT_FAILURE);
    }
                
    unsigned int* addr = (unsigned int*) s->regs[instr->rn];
    
    if (instr->l == 0 && instr->p == 1) { // Push (stmfd, stmdb)
        for (int i = 0 ; i < 17 ; i++) {
            if ( ( instr->regs & (1 << i) ) >> i == 1) {
                debug("Store r%d (value %d) to address 0x%02x at r%d", i, s->regs[i], (unsigned int) addr, instr->rn);
                *addr = s->regs[i];
                addr--;
            }
        } // end for
    } else if (instr->l == 1 && instr->p == 0) { // Pop (ldmfd, ldmia)
        for (int i = 16 ; i >= 0 ; i--) {
            if ( ( instr->regs & (1 << i) ) >> i == 1) {
                addr++; // First move SP (or whatever Rn is)
                debug("Copy value %d from address 0x%02x (r%d) to r%d", *addr, (unsigned int) addr, instr->rn, i);
                s->regs[i] = *addr;
            }
        } // end for
    } // end if push or pop

    s->regs[instr->rn] = (unsigned int) addr;
} // end armemu_one_load_store_multiple

