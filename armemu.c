#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 1
#include "debug_utils.h"

unsigned int add_function (unsigned int a, unsigned int b, unsigned int c, unsigned int d);

#define NUM_REGS 16
#define SP 13
#define LR 14
#define PC 15
#define STACK_SIZE 4096

struct state {
    unsigned int regs[NUM_REGS];
    unsigned int cpsr;
    unsigned int stack[STACK_SIZE];
};

unsigned int select_bits(unsigned int src, int high, int low)
{
    unsigned int mask = 0xFFFFFFFF;
    int nb_ones = 1 + high - low;

    mask >>= 32 - nb_ones;

    int shift = high - nb_ones + 1;
    mask <<= shift;
    return (mask & src) >> shift;
}

// Data Processing, Src2, immediate
struct dp_src2_immediate {
    unsigned int rot  : 4 ;
    unsigned int imm8 : 8 ;
};

// Data Processing, Src2, (Constant shifted) register
struct dp_src2_reg {
    unsigned int shamt5       : 5 ;
    unsigned int sh           : 2 ;
    unsigned int is_reg_shift : 1 ; // set to 0 here because shifted by a constant
    unsigned int rm           : 4 ;
};

// Data Processing, Src2, Register shifted by another register
struct dp_src2_reg_shifted {
    unsigned int rs           : 4 ;
    unsigned int              : 1 ; // unused
    unsigned int sh           : 2 ;
    unsigned int is_reg_shift : 1 ; // set to 1 here because shifted by a register
    unsigned int rm           : 4 ;
};

// Data processing instruction
// DDCA, page 330
// Due to endianness, wasn't able to use the bitfield directly
struct dp_instr {
    unsigned int cond  :  4  ;
    unsigned int op    :  2  ;
    unsigned int i     :  1  ;
    unsigned int cmd   :  4  ;
    unsigned int s     :  1  ;
    unsigned int rn    :  4  ;
    unsigned int rd    :  4  ;
    unsigned int src2  : 12  ;
};

struct dp_instr decode_dp_instr (unsigned int raw)
{
    return (struct dp_instr) {
        .cond   =     select_bits(raw, 31, 28) ,
        .op     =     select_bits(raw, 27, 26) ,
        .i      =     select_bits(raw, 25, 25) ,
        .cmd    =     select_bits(raw, 24, 21) ,
        .s      =     select_bits(raw, 20, 20) ,
        .rn     =     select_bits(raw, 19, 16) ,
        .rd     =     select_bits(raw, 15, 12) ,
        .src2   =     select_bits(raw, 11, 0) 
    };
}

// Assembly function signature
typedef unsigned int (*func)(unsigned int, unsigned int, unsigned int, unsigned int);

void init_state(struct state* s, 
                func f, 
                unsigned int r0, 
                unsigned int r1,
                unsigned int r2,
                unsigned int r3)
{
    s->regs[0] = r0;
    s->regs[1] = r1;
    s->regs[2] = r2;
    s->regs[3] = r3;

    for (int i = 4 ; i < NUM_REGS ; ++i) {
        s->regs[i] = 0;
    }

    s->cpsr = 0;

    s->regs[SP] = (unsigned int) &(s->stack[STACK_SIZE+1]);
    s->regs[PC] = (unsigned int) f;
    s->regs[LR] = 0;

    debug("SP = 0x%02x", s->regs[SP]);
    debug("PC = 0x%02x", s->regs[PC]);
    debug("LR = 0x%02x", s->regs[LR]);
}

int rotate_immediate (int rot, int imm8)
{
    rot *= 2;
    return (imm8 << rot)|(imm8 >> (32 - rot));
}

struct dp_src2_immediate decode_dp_src2_immediate (unsigned int raw)
{
    return (struct dp_src2_immediate) {
        .rot  = select_bits(raw, 11, 8),
        .imm8 = select_bits(raw,  7, 0)
    };
}

struct dp_src2_reg decode_dp_src2_reg (unsigned int raw)
{
    return (struct dp_src2_reg) {
        .shamt5       = select_bits(raw, 11, 7),
        .sh           = select_bits(raw,  6, 5),
        .is_reg_shift = select_bits(raw,  4, 4),
        .rm           = select_bits(raw,  3, 0),
    };
}

void add_or_subtract(struct state* state, struct dp_instr* inst, bool is_add)
{
    int src2_value = 0 ;
    if (inst->i == 1) {
        struct dp_src2_immediate imm = decode_dp_src2_immediate(inst->src2);
        src2_value = rotate_immediate(imm.rot, imm.imm8);
    } else {
        struct dp_src2_reg reg = decode_dp_src2_reg(inst->src2);
        if (reg.is_reg_shift) {
            fprintf(stderr, "Src2 register shifted is not supported yet.\n");
            exit(EXIT_FAILURE);
        } else {
            if (reg.shamt5 != 0) {
                fprintf(stderr, "Src2 shifts are not supported at this time. \n");
                exit(EXIT_FAILURE);
            }
            src2_value = (int) state->regs[reg.rm];
        }
    }
    if (!is_add) {
        src2_value = -src2_value;
    }
    debug("Add %d to r%d", src2_value, inst->rn);
    state->regs[inst->rd] = (int) (state->regs[inst->rn]) + src2_value;
}

void branch_and_exchange(struct state* state, struct dp_instr* inst)
{
    unsigned int rn = select_bits(inst->src2, 3, 0);
    unsigned int rn_val = state->regs[rn];
    debug("Current value of PC: 0x%02x", state->regs[PC]);
    debug("Current value of LR: 0x%02x", state->regs[LR]);
    state->regs[PC] = rn_val;
    debug("Update PC to 0x%02x", rn_val);
}

void print_instr(struct dp_instr* i)
{
    printf("cond: %u\n"
           "  op: %u\n"
           "   I: %u\n"
           " cmd: %u\n"
           "   S: %u\n"
           "  Rn: %u\n"
           "  Rd: %u\n"
           "Src2: %u\n",
           i->cond,
           i->op,
           i->i,
           i->cmd,
           i->s,
           i->rn,
           i->rd,
           i->src2
           );
}

void mov(struct state* state, struct dp_instr* instr)
{
    // rd = src2
    int value;
    if (instr->i == 1) {
        debug("Immediate value %d", instr->src2);
        value = instr->src2;
    } else {
        unsigned int shift = select_bits(instr->src2, 11, 8);
        unsigned int reg = select_bits(instr->src2, 7, 0);
        value = state->regs[reg];
        debug("Value (%d) from r%d shifted by %d", value, reg, shift);
        if (shift != 0) {
            value <<= shift;
            debug("Shifted value: %d", value);
        }
    }

    state->regs[instr->rd] = value;
}

void armemu_one_dp(struct state* state, struct dp_instr* inst)
{
    switch (inst->cmd) {
        case 0x2:
            add_or_subtract(state, inst, false);
            break;
        case 0x4:
            add_or_subtract(state, inst, true);
            break;
        case 0x9:
            branch_and_exchange(state, inst);
            break;
        case 0xd:
            mov(state, inst);
            break;
        default:
            fprintf(stderr, "Unknown Data Processing instruction with cmd %02x.\n",
                    inst->cmd);
            exit(EXIT_FAILURE);
    }

}

struct branch_link_instr {
    unsigned int link   :  1 ;
    unsigned int offset   : 24 ;
};

struct branch_link_instr decode_branch_link_instr (unsigned int raw)
{
    return (struct branch_link_instr) {
        .link   = select_bits(raw, 24, 24),
        .offset = select_bits(raw, 23, 0)
    };
}

void armemu_one_branch(struct state* state, struct branch_link_instr* instr)
{
    if (instr->link) {
        debug("Is link instruction, LR = PC (0x%02x) + 4 = 0x%02x", state->regs[PC], state->regs[PC]+4); 
        state->regs[LR] = state->regs[PC] + 4;
    }

    unsigned int raw_offset = instr->offset;
    raw_offset <<= 2;
    
    // Sign extend offset
    if (select_bits(raw_offset, 25, 25) == 1) {
        raw_offset |= 0xFC000000;
    }

    signed int signed_offset = (signed int) raw_offset;

    state->regs[PC] = state->regs[PC] + 8 + signed_offset; // 8 simulates prefetch
    debug("Branch to 0x%02x", state->regs[PC]);
}

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

void armemu_one (struct state* s)
{
    unsigned int* pc_addr = (unsigned int*) s->regs[PC];
    debug("PC is at address 0x%02x", s->regs[PC]);
    struct dp_instr dp_instr = decode_dp_instr(*pc_addr);
    //print_instr(&dp_instr);
    
    if (condition_is_true(s, dp_instr.cond)) {
        debug("Condition %d is true", dp_instr.cond);

        switch (dp_instr.op) {
            case 0x00: // Data processing
                armemu_one_dp(s, &dp_instr);
                break;
            case 0x01: // Data transfer (LDR, STR)
                {
                    struct load_store_instr instr = decode_load_store_instr(*pc_addr);
                    armemu_one_load_store(s, &instr);
                    break;
                }
            case 0x02: // Branch and link or STM/LDM
                { 
                    unsigned int code = select_bits(*pc_addr, 27, 25);
                    if (code == 0x5) { // Branch
                        struct branch_link_instr bl_instr = decode_branch_link_instr(*pc_addr);
                        armemu_one_branch(s, &bl_instr);
                    } else if (code == 0x4) { // STM/LDM
                        struct load_store_multiple_instr lsm_instr = decode_load_store_multiple_instr(*pc_addr);
                        armemu_one_load_store_multiple(s, &lsm_instr);
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
    } else {
        debug("Condition %d is false", dp_instr.cond);
    }

    // Update PC
    if (dp_instr.rn != PC) {
        s->regs[PC] = s->regs[PC] + 4;
    }
}

void armemu(struct state* s)
{
    int i = 0;
    while (s->regs[PC] != 0) {
        debug("Instruction #%d", ++i); 
        armemu_one(s); 
    }
    debug("Reached function end. %d instructions executed.", i);
}

func find_func (char* name)
{
    if (strcmp(name, "add_function") == 0) return add_function;
    
    return NULL;
}

int main (int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s FUNCTION\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct state state = { };

    func func = find_func(argv[1]);
    if (!func) {
        fprintf(stderr, "Function %s could not be found.\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    init_state(&state, func, 10, 11, 12, 13);
    armemu(&state);
    printf("r = %d\n", state.regs[0]);
}

