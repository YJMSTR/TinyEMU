#ifndef __CPU_H__
#define __CPU_H__
#include "common.h"

enum INST_NAME {
    //RV64I
    LUI,
    AUIPC,
    JAL,
    JALR,
    BEQ,
    BNE,
    BLT,
    BGE,
    BLTU,
    BGEU,
    LB,
    LH,
    LW,
    LBU,
    LHU,
    SB,
    SH,
    SW,
    ADDI,
    SLTI,
    SLTIU,
    XORI,
    ORI,
    ANDI,
    SLLI,
    SRLI,
    SRAI,
    ADD,
    SUB,
    SLL,
    SLT,
    SLTU,
    XOR,
    SRL,
    SRA,
    OR,
    AND,
    FENCE,
    ECALL,
    EBREAK,
    LWU,
    LD,
    SD,
    ADDIW,
    SLLIW,
    SRLIW,
    SRAIW,
    ADDW,
    SUBW,
    SLLW,
    SRLW,
    SRAW,
    INST_NUM,
};

enum CPU_STATE {
    CPU_STOP,
    CPU_RUN,
};

typedef struct CPU {
    uint64_t regs[32];
    uint64_t pc;
    enum CPU_STATE state;
} CPU;

extern CPU cpu;

typedef struct DECODER {
    uint32_t inst_val;
    enum INST_NAME inst_name;
    uint64_t dest;
    uint64_t src1;
    uint64_t src2;
    uint64_t imm;
    int rd;
    int rs1;
    int rs2;
    int shamt;
    uint64_t snpc;
    uint64_t dnpc;
    CPU *cpu;
} DECODER;

extern void (*inst_handle[INST_NUM])(DECODER*);

void init_inst_func();

uint64_t MASK(int n);
uint64_t BITS(uint64_t imm, int hi, int lo);
uint64_t SEXT(uint64_t imm, int n);

void cpu_init(CPU *cpu);
uint64_t inst_fetch(CPU *cpu);
void cpu_exec(CPU *cpu, uint32_t n);
void exec_once(CPU *cpu);
DECODER decode(uint32_t inst);

#endif