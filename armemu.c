#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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

// Data Processing operand Src2
union dp_src2 {
    struct dp_src2_immediate    immediate;
    struct dp_src2_reg          reg;
    struct dp_src2_reg_shifted  reg_shifted;
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

void init_state(struct state* s, func f, 
                unsigned int r0, 
                unsigned int r1,
                unsigned int r2,
                unsigned int r3)
{
    printf("%d %d %d %d\n", r0, r1, r2, r3);

    for (int i = 0 ; i < NUM_REGS ; ++i) {
        s->regs[i] = 0;
    }

    s->cpsr = 0;

    s->regs[SP] = (unsigned int) &(s->stack[STACK_SIZE+1]);
    s->regs[PC] = (unsigned int) f;
    s->regs[LR] = 0;
}

int rotate_immediate (int rot, int imm8)
{
    rot *= 2;
    return (imm8 << rot)|(imm8 >> (32 - rot));
}

void add(struct state* state, struct dp_instr* inst)
{
    int src2_value = 0 ;
    union dp_src2 src2 = (union dp_src2) inst->src2;
    if (inst->i == 1) {
        struct dp_src2_immediate imm = (struct dp_src2_immediate) src2;
        src2_value = rotate_immediate(imm->rot, imm->imm8);
    } else {
        struct dp_src2_reg reg = src2.reg_shifted;
        if (reg->is_reg_shift) {
            fprintf(stderr, "Src2 register shifted is not supported yet.\n");
            exit(EXIT_FAILURE);
        } else {
            if (reg->shamt5 != 0) {
                fprintf(stderr, "Src2 shifts are not supported at this time. \n");
                exit(EXIT_FAILURE);
            }
            src2_value = (int) reg.rm;
        }
    }
    state->regs[inst->rd] = (int) (state->regs[inst->rn]) + src2_value;
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

void armemu_one_dp(struct state* state, struct dp_instr* inst)
{
    switch (inst->cmd) {
        case 0x4:
            add(state, inst);
            break;
        default:
            fprintf(stderr, "Unknown Data Processing instruction with cmd %02x.\n",
                    inst->cmd);
    }

}

void armemu_one(struct state* s)
{
    unsigned int* pc_addr = (unsigned int*) s->regs[PC];
    struct dp_instr dp_instr = decode_dp_instr(*pc_addr);
    print_instr(&dp_instr);

    switch (dp_instr.op) {
        case 0x00: // Data processing
            armemu_one_dp(s, &dp_instr);
            break;
        default:
            fprintf(stderr, "Unknown instruction type (op) %02x.\n",
                    dp_instr.op);
    }

    // Update PC
    if (dp_instr.rn != PC) {
        s->regs[PC] = s->regs[PC] + 4;
    }
}

void armemu(struct state* s)
{
    int i = 0;
//    while(s->regs[PC] != 0) {
    for ( ; i < 4 ; ++i) {
        printf("%d:\n", i); armemu_one(s); 
    }
}


int main (int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s FUNCTION\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct state state = { };

    init_state(&state, add, 1, 2, 3, 4);
    armemu(&state);
    // printf("r = %d\n", r);
}
