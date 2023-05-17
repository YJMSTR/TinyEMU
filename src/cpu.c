#include "../includes/cpu.h"
#include "../includes/trap.h"
#include "../includes/mem.h"
#include "../includes/bus.h"
#include "../includes/common.h"

uint64_t p_addr;
bool riscv_tests;
const char *reg_abinames[] = {"$0","ra","sp","gp","tp","t0","t1","t2","s0","s1","a0","a1","a2","a3","a4","a5","a6","a7","s2","s3","s4","s5","s6","s7","s8","s9","s10","s11","t3","t4","t5","t6"};
CPU cpu;
void (*inst_handle[INST_NUM])(DECODER*);

uint64_t MASK(uint64_t n) {
    if (n == 64) return ~0ull;
    return (1ull << n) - 1ull;
}
uint64_t BITS(uint64_t imm, uint64_t hi, uint64_t lo) {
    return (imm >> lo) & MASK(hi - lo + 1ull);
}
uint64_t SEXT(uint64_t imm, uint64_t n) {
    if ((imm >> (n-1)) & 1) {
        printf("the src and res of sext are 0x%llx 0x%llx\n", imm, ((~0ull) << n) | imm);
        return ((~0ull) << n) | imm;
    } else return imm & MASK(n);
}
uint64_t imm_u(uint32_t inst) {return SEXT(BITS(inst, 31, 12), 20);}
uint64_t imm_j(uint32_t inst) {return (SEXT(BITS(inst, 31, 31), 1) << 20) | (BITS(inst, 30, 21) << 1) | (BITS(inst, 20, 20) << 11) | (BITS(inst, 19, 12) << 12);}
uint64_t imm_i(uint32_t inst) {return SEXT(BITS(inst, 31, 20), 12);}
uint64_t imm_s(uint32_t inst) {return SEXT((BITS(inst, 31, 25) << 5) | BITS(inst, 11, 7), 12); }
uint64_t imm_b(uint32_t inst) {return (SEXT(BITS(inst, 31, 31), 1) << 12) | (BITS(inst, 30, 25) << 5) | (BITS(inst, 11, 8) << 1) | (BITS(inst, 7, 7) << 11);}


void cpu_init(CPU *cpu) {
    cpu->pc = RESET_VECTOR;
    cpu->regs[0] = 0;   
    cpu->regs[2] = DRAM_BASE + DRAM_SIZE;
    cpu->state = CPU_RUN;
    cpu->pri_level = M;
}

uint64_t cpu_load(CPU *cpu, uint64_t addr, int length) {
    return mem_load(&cpu->bus.dram, addr, length);
}

void cpu_store(CPU *cpu, uint64_t addr, int length, uint64_t val) {
    mem_store(&cpu->bus.dram, addr, length, val);
}

uint64_t inst_fetch(CPU *cpu) {
    return cpu_load(cpu, cpu->pc, 4);
}

void cpu_exec(CPU *cpu, uint32_t n) {
    for (int i = 0; i < n; i++) {
        if (cpu->state == CPU_RUN)
            exec_once(cpu);
        else break;
    }
}

void exec_once(CPU *cpu) {
    cpu->regs[0] = 0;
    if (cpu->pc == p_addr) {
        cpu->state = CPU_STOP;
        printf("breakpoint at 0x%lx\n", p_addr);
        p_addr = -1;
        return;
    }
    uint32_t inst = inst_fetch(cpu);
    printf("inst = 0x%08x\n", inst);
    DECODER decoder = decode(inst);
    if (decoder.inst_name == INST_NUM) {
        cpu->state = CPU_STOP;
        printf("Unsupported instruction or EOF\n");
        printf("TinyEMU STOP\n");
        printf("PC = 0x%08lx inst=0x%08x inst_name = %d\n", cpu->pc, inst, decoder.inst_name);
        return;
    }
    decoder.dnpc = decoder.snpc = cpu->pc + 4;
    decoder.cpu = cpu;
    inst_handle[decoder.inst_name](&decoder);
    cpu->pc = decoder.dnpc;
}


DECODER decode(uint32_t inst) {
    DECODER ret;
    ret.inst_val = inst;
    ret.rd = BITS(inst, 11, 7);
    ret.rs1 = BITS(inst, 19, 15);
    ret.rs2 = BITS(inst, 24, 20);
    ret.shamt = BITS(inst, 25, 20);
    ret.csr_addr = BITS(inst, 31, 20);
    uint32_t funct3 = BITS(inst, 14, 12);
    uint32_t funct7 = BITS(inst, 31, 25);
    uint32_t funct6 = BITS(inst, 31, 26);
    uint32_t opcode = BITS(inst, 6, 0);
    switch (opcode) {
        case 0b0110111:
            ret.inst_name = LUI;
            ret.imm = imm_u(inst);
            break;
        case 0b0010111:
            ret.inst_name = AUIPC;
            ret.imm = imm_u(inst);
            break;
        case 0b1101111:
            ret.inst_name = JAL;
            ret.imm = imm_j(inst);
            break;
        case 0b1100111:
            ret.inst_name = JALR;
            ret.imm = imm_i(inst);
            break;
        case 0b1100011:
            ret.imm = imm_b(inst);
            switch (funct3) {
                case 0b000:
                    ret.inst_name = BEQ;
                    break;
                case 0b001:
                    ret.inst_name = BNE;
                    break;
                case 0b100:
                    ret.inst_name = BLT;
                    break;
                case 0b101:
                    ret.inst_name = BGE;
                    break;
                case 0b110:
                    ret.inst_name = BLTU;
                    break;
                case 0b111:
                    ret.inst_name = BGEU;
                    break;
            }
            break;
        case 0b0000011:
            ret.imm = imm_i(inst);
            switch (funct3) {
                case 0b000:
                    ret.inst_name = LB;
                    break;
                case 0b001:
                    ret.inst_name = LH;
                    break;
                case 0b010:
                    ret.inst_name = LW;
                    break;
                case 0b100:
                    ret.inst_name = LBU;
                    break;
                case 0b101:
                    ret.inst_name = LHU;
                    break;
                case 0b110:
                    ret.inst_name = LWU;
                    break;
                case 0b011:
                    ret.inst_name = LD;
                    break;
            }
            break;
        case 0b0100011:
            ret.imm = imm_s(inst);
            printf("imm_s == 0x%llx\n", ret.imm);
            switch (funct3) {
                case 0b000:
                    ret.inst_name = SB;
                    break;
                case 0b001:
                    ret.inst_name = SH;
                    break;
                case 0b010:
                    ret.inst_name = SW;
                    break;
                case 0b011:
                    ret.inst_name = SD;
                    break;
            }   
            break;
        case 0b0010011:
            ret.imm = imm_i(inst);
            switch (funct3) {
                case 0b000:
                    ret.inst_name = ADDI;
                    break;
                case 0b010:
                    ret.inst_name = SLTI;
                    break;
                case 0b011:
                    ret.inst_name = SLTIU;
                    break;
                case 0b100:
                    ret.inst_name = XORI;
                    break;
                case 0b110:
                    ret.inst_name = ORI;
                    break;
                case 0b111:
                    ret.inst_name = ANDI;
                    break;
                case 0b001:
                    ret.inst_name = SLLI;
                    break;
                case 0b101:
                    switch (funct6) {
                        case 0:
                            ret.inst_name = SRLI;
                            break;
                        case 0b10000:
                            ret.inst_name = SRAI;
                            break;
                    } 
                    break;
            }
            break;
        case 0b0110011:
            switch ((funct7 << 3) | funct3) {
                case 0:
                    ret.inst_name = ADD;
                    break;
                case 0b100000000:
                    ret.inst_name = SUB;
                    break;
                case 0b1:
                    ret.inst_name = SLL;
                    break;
                case 0b10:
                    ret.inst_name = SLT;
                    break;
                case 0b11:
                    ret.inst_name = SLTU;
                    break;
                case 0b100:
                    ret.inst_name = XOR;
                    break;
                case 0b101:
                    ret.inst_name = SRL;
                    break;
                case 0b100000101:
                    ret.inst_name = SRA;
                    break;
                case 0b110:
                    ret.inst_name = OR;
                    break;
                case 0b111:
                    ret.inst_name = AND;
                    break;
            }    
            break;
        case 0b0001111:
            switch (funct3) {
                case 0:
                    ret.inst_name = FENCE;
                    break;
                case 1:
                    //fence_i
                    //implement as NOP
                    ret.inst_name = FENCE;
                    break;
                default:
                    ret.inst_name = INST_NUM;
                    break;
            }
            break;
        case 0b1110011:
            if (funct3 != 0) {
                // csr
                switch (funct3) {
                    case 0b001:
                        ret.inst_name = CSRRW;
                        break;
                    case 0b010:
                        ret.inst_name = CSRRS;
                        break;
                    case 0b011: 
                        ret.inst_name = CSRRC;
                        break;
                    case 0b101:
                        ret.inst_name = CSRRWI;
                        break;
                    case 0b110:
                        ret.inst_name = CSRRSI;
                        break;
                    case 0b111:
                        ret.inst_name = CSRRCI;
                        break;
                    default:
                        ret.inst_name = INST_NUM;
                        break;
                }
            } else {
                switch ((funct7 << 5) | ret.rs2) {
                    case 0:
                        ret.inst_name = ECALL;
                        break;
                    case 1:
                        ret.inst_name = EBREAK;
                        break;
                    case 0b100000010:
                        ret.inst_name = SRET;
                        break;
                    case 0b1100000010:
                        ret.inst_name = MRET;
                        break;
                    default:
                        ret.inst_name = INST_NUM;
                        break;
                }
            }
            
            break;
        case 0b0011011:
            switch (funct3) {
                case 0:
                    ret.imm = imm_i(inst);
                    ret.inst_name = ADDIW;
                    break;
                case 0b001:
                    switch (funct7) {
                        case 0:
                            ret.inst_name = SLLIW;
                            break;
                        default:
                            ret.inst_name = INST_NUM;
                            break;
                    }
                    break;
                case 0b101:
                    switch (funct7) {
                        case 0:
                            ret.inst_name = SRLIW;
                            break;
                        case 0b100000:
                            ret.inst_name = SRAIW;
                            break;
                    }
                    break;
                default:
                    ret.inst_name = INST_NUM;
                    break;
            }
            break;
        case 0b0111011:
            switch ((funct7 << 3) | funct3) {
                case 0b0:
                    ret.inst_name = ADDW;
                    break;
                case 0b0100000000:
                    ret.inst_name = SUBW;
                    break;
                case 0b1:
                    ret.inst_name = SLLW;
                    break;
                case 0b101:
                    ret.inst_name = SRLW;
                    break;
                case 0b100000101:
                    ret.inst_name = SRAW;
                    break;
                default:
                    ret.inst_name = INST_NUM;
                    break;
            }
            break;
        default:
            ret.inst_name = INST_NUM;
            return ret;
    }
    return ret;
}

void set_inst_func(enum INST_NAME inst_name, void (*fp)(DECODER*)) {
    inst_handle[inst_name] = fp;
}

void lui(DECODER *decoder) { 
    printf("lui decoder->imm = %llx\n", decoder->imm);
    decoder->cpu->regs[decoder->rd] = decoder->imm << 12;
    printf("lui decoder->imm << 12 = %llx\n", decoder->cpu->regs[decoder->rd]);
}

void auipc(DECODER *decoder) { 
    decoder->cpu->regs[decoder->rd] = decoder->cpu->pc + (decoder->imm << 12);
}

void jal(DECODER *decoder) { 
    decoder->cpu->regs[decoder->rd] = decoder->snpc; 
    decoder->dnpc = decoder->imm + decoder->cpu->pc;
}

void jalr(DECODER *decoder) { 
    decoder->dnpc = (decoder->cpu->regs[decoder->rs1] + decoder->imm) & ~1; 
    decoder->cpu->regs[decoder->rd] = decoder->snpc;
}

void beq(DECODER *decoder) { 
    if (decoder->cpu->regs[decoder->rs1] == decoder->cpu->regs[decoder->rs2]) {
        printf("beq offset = 0x%lx\n", decoder->imm);
        decoder->dnpc = decoder->cpu->pc + decoder->imm;
    }
}

void bne(DECODER *decoder) {
    if (decoder->cpu->regs[decoder->rs1] != decoder->cpu->regs[decoder->rs2]) {
        printf("bne offset = 0x%lx, rs1 = 0x%lx, rs2 = 0x%lx\n", decoder->imm, decoder->cpu->regs[decoder->rs1], decoder->cpu->regs[decoder->rs2]);
        decoder->dnpc = decoder->cpu->pc + decoder->imm;
    }
}

void blt(DECODER *decoder) {
    if ((long long)decoder->cpu->regs[decoder->rs1] < (long long)decoder->cpu->regs[decoder->rs2]) {
        decoder->dnpc = decoder->cpu->pc + decoder->imm;
    }
}

void bge(DECODER *decoder) {
    if ((long long)decoder->cpu->regs[decoder->rs1] >= (long long)decoder->cpu->regs[decoder->rs2]) {
        decoder->dnpc = decoder->cpu->pc + decoder->imm;
    }
}

void bltu(DECODER *decoder) {
    if (decoder->cpu->regs[decoder->rs1] < decoder->cpu->regs[decoder->rs2]) {
        decoder->dnpc = decoder->cpu->pc + decoder->imm;
    }
}

void bgeu(DECODER *decoder) {
    if (decoder->cpu->regs[decoder->rs1] >= decoder->cpu->regs[decoder->rs2]) {
        decoder->dnpc = decoder->cpu->pc + decoder->imm;
    }
}

void lb(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = SEXT(cpu_load(decoder->cpu, decoder->cpu->regs[decoder->rs1] + decoder->imm, 1), 8);
}

void lh(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = SEXT(cpu_load(decoder->cpu, decoder->cpu->regs[decoder->rs1] + decoder->imm, 2), 16);
}

void lw(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = SEXT(cpu_load(decoder->cpu, decoder->cpu->regs[decoder->rs1] + decoder->imm, 4), 32);
}

void lbu(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = cpu_load(decoder->cpu, decoder->cpu->regs[decoder->rs1] + decoder->imm, 1);
}

void lhu(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = cpu_load(decoder->cpu, decoder->cpu->regs[decoder->rs1] + decoder->imm, 2);
}

void lwu(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = cpu_load(decoder->cpu, decoder->cpu->regs[decoder->rs1] + decoder->imm, 4);
}

void ld(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = cpu_load(decoder->cpu, decoder->cpu->regs[decoder->rs1] + decoder->imm, 8);
}

void sb(DECODER *decoder) {
    cpu_store(decoder->cpu, decoder->cpu->regs[decoder->rs1] + decoder->imm, 1, decoder->cpu->regs[decoder->rs2]);
}

void sh(DECODER *decoder) {
    cpu_store(decoder->cpu, decoder->cpu->regs[decoder->rs1] + decoder->imm, 2, decoder->cpu->regs[decoder->rs2]);
}

void sw(DECODER *decoder) {
    cpu_store(decoder->cpu, decoder->cpu->regs[decoder->rs1] + decoder->imm, 4, decoder->cpu->regs[decoder->rs2]);
}

void sd(DECODER *decoder) {
    printf("sd addr = 0x%llx\n", decoder->cpu->regs[decoder->rs1] + decoder->imm);
    cpu_store(decoder->cpu, decoder->cpu->regs[decoder->rs1] + decoder->imm, 8, decoder->cpu->regs[decoder->rs2]);
}

void addi(DECODER *decoder) {
    printf("addi rd = %d x[rs1 = %d] = 0x%lx imm = 0x%lx\n", decoder->rd, decoder->rs1, decoder->cpu->regs[decoder->rs1], decoder->imm);
    decoder->cpu->regs[decoder->rd] = decoder->cpu->regs[decoder->rs1] + decoder->imm;
}

void slti(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = (long long)decoder->cpu->regs[decoder->rs1] < (long long)decoder->imm ? 1 : 0;
}

void sltiu(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = decoder->cpu->regs[decoder->rs1] < decoder->imm ? 1 : 0;
}

void xori(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = decoder->cpu->regs[decoder->rs1] ^ decoder->imm;
}

void ori(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = decoder->cpu->regs[decoder->rs1] | decoder->imm;
}

void andi(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = decoder->cpu->regs[decoder->rs1] & decoder->imm;
}

void slli(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = decoder->cpu->regs[decoder->rs1] << decoder->shamt;
}

void srli(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = decoder->cpu->regs[decoder->rs1] >> decoder->shamt;
}

void srai(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = ((long long)decoder->cpu->regs[decoder->rs1]) >> decoder->shamt;
}

void add(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = decoder->cpu->regs[decoder->rs1] + decoder->cpu->regs[decoder->rs2];
}

void sub(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = decoder->cpu->regs[decoder->rs1] - decoder->cpu->regs[decoder->rs2];
}

void sll(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = decoder->cpu->regs[decoder->rs1] << BITS(decoder->cpu->regs[decoder->rs2], 5, 0);
}

void slt(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = (long long)decoder->cpu->regs[decoder->rs1] < (long long)decoder->cpu->regs[decoder->rs2] ? 1 : 0;
}

void sltu(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = decoder->cpu->regs[decoder->rs1] < decoder->cpu->regs[decoder->rs2] ? 1 : 0;
}

void xor(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = decoder->cpu->regs[decoder->rs1] ^ decoder->cpu->regs[decoder->rs2];
}

void srl(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = decoder->cpu->regs[decoder->rs1] >> BITS(decoder->cpu->regs[decoder->rs2], 5, 0);
}

void sra(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = ((long long)decoder->cpu->regs[decoder->rs1]) >> BITS(decoder->cpu->regs[decoder->rs2], 5, 0);
}

void or(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = decoder->cpu->regs[decoder->rs1] | decoder->cpu->regs[decoder->rs2];
}

void and(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = decoder->cpu->regs[decoder->rs1] & decoder->cpu->regs[decoder->rs2];
}

void fence(DECODER *decoder) {
    //todo
    return;
}

void ecall(DECODER *decoder) {
    if (riscv_tests && decoder->cpu->regs[a7] == 93) {
        if (decoder->cpu->regs[a0] == 0) {
            printf("Test Pass\n");
            decoder->cpu->state = CPU_STOP;
        } else {
            printf("Test #%d Fail\n", decoder->cpu->regs[a0]/2);
            decoder->cpu->state = CPU_STOP;
        }
    }
    trap_handler(decoder, Requested, false, decoder->cpu->pri_level + 8, 0);
    return;
}

void ebreak(DECODER *decoder) {
    //todo
    exit(0);
    return;
}

void addiw(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = SEXT(BITS(decoder->cpu->regs[decoder->rs1] + decoder->imm, 31, 0), 32);
}

void slliw(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = SEXT(BITS(decoder->cpu->regs[decoder->rs1] << decoder->shamt, 31, 0), 32);
}

void srliw(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = SEXT(BITS(decoder->cpu->regs[decoder->rs1], 31, 0) >> decoder->shamt, 32);
}

void sraiw(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = SEXT(((int)BITS(decoder->cpu->regs[decoder->rs1], 31, 0)) >> decoder->shamt, 32);
}

void addw(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = SEXT(BITS(decoder->cpu->regs[decoder->rs1] + decoder->cpu->regs[decoder->rs2], 31, 0), 32);
}

void subw(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = SEXT(decoder->cpu->regs[decoder->rs1] - decoder->cpu->regs[decoder->rs2], 32);
}

void sllw(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = SEXT(BITS(decoder->cpu->regs[decoder->rs1] << BITS(decoder->cpu->regs[decoder->rs2], 4, 0), 31, 0), 32);
}

void srlw(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = SEXT(BITS(decoder->cpu->regs[decoder->rs1], 31, 0) >> BITS(decoder->cpu->regs[decoder->rs2], 4, 0), 32);
}

void sraw(DECODER *decoder) {
    decoder->cpu->regs[decoder->rd] = SEXT((int)BITS(decoder->cpu->regs[decoder->rs1], 31, 0) >> BITS(decoder->cpu->regs[decoder->rs2], 4, 0), 32);
}

//Zicsr
void csrrw(DECODER *decoder) {
    uint64_t csrval;
    if (decoder->rd != 0) 
        csrval = decoder->cpu->csr.csr[decoder->csr_addr];
    else csrval = 0;
    uint64_t rs1val = decoder->cpu->regs[decoder->rs1];
    printf("csrval = 0x%08lx, rs1val = 0x%08lx\n", csrval, rs1val);
    decoder->cpu->regs[decoder->rd] = csrval;
    decoder->cpu->csr.csr[decoder->csr_addr] = rs1val;
}

void csrrs(DECODER *decoder) {
    uint64_t csrval = decoder->cpu->csr.csr[decoder->csr_addr];
    uint64_t rs1val = decoder->rs1 == 0 ? 0 : decoder->cpu->regs[decoder->rs1];
    printf("before csrval = 0x%08lx, rs1val = 0x%08lx\n", csrval, rs1val);
    decoder->cpu->regs[decoder->rd] = csrval;
    if (decoder->rs1 != 0) 
        decoder->cpu->csr.csr[decoder->csr_addr] = csrval | rs1val;
    printf("after csrval = 0x%08lx, rs1val = 0x%08lx\n", decoder->cpu->csr.csr[decoder->csr_addr], rs1val);
}

void csrrc(DECODER *decoder) {
    uint64_t csrval = decoder->cpu->csr.csr[decoder->csr_addr];
    uint64_t rs1val = decoder->rs1 == 0 ? 0 : decoder->cpu->regs[decoder->rs1];
    decoder->cpu->regs[decoder->rd] = csrval;
    rs1val = ~rs1val;
    if (decoder->rs1 != 0)
        decoder->cpu->csr.csr[decoder->csr_addr] = csrval & rs1val;
}

void csrrwi(DECODER *decoder) {
    uint64_t uimm = decoder->rs1;
    uint64_t csrval = decoder->rd == 0 ? 0 : decoder->cpu->csr.csr[decoder->csr_addr];
    decoder->cpu->regs[decoder->rd] = csrval;
    decoder->cpu->csr.csr[decoder->csr_addr] = uimm;
}

void csrrsi(DECODER *decoder) {
    uint64_t uimm = decoder->rs1;
    uint64_t csrval = decoder->cpu->csr.csr[decoder->csr_addr];
    decoder->cpu->regs[decoder->rd] = csrval;
    decoder->cpu->csr.csr[decoder->csr_addr] = csrval | uimm;
}

void csrrci(DECODER *decoder) {
    uint64_t uimm = decoder->rs1;
    uint64_t csrval = decoder->cpu->csr.csr[decoder->csr_addr];
    decoder->cpu->regs[decoder->rd] = csrval;
    uimm = ~uimm;
    decoder->cpu->csr.csr[decoder->csr_addr] = csrval & uimm;
}

//trap return inst
void mret(DECODER *decoder) {
    int pre_level = get_xpp(M);
    set_xie(M, get_xpie(M));
    set_xpie(M, 1);
    set_xpp(M, U);
    set_csr(mstatus, get_csr(mstatus) & (~(1 << 17)));
    decoder->dnpc = get_csr(mepc);
    cpu.pri_level = pre_level;
}

void sret(DECODER *decoder) {
    int pre_level = get_xpp(S);
    set_xie(S, get_xpie(S));
    set_xpie(S, 1);
    set_xpp(S, U);
    set_csr(sstatus, get_csr(sstatus) & (~(1 << 17)));
    decoder->dnpc = get_csr(sepc);
    cpu.pri_level = pre_level;
}

void init_inst_func() {
    set_inst_func(LUI, lui);
    set_inst_func(AUIPC, auipc);
    set_inst_func(JAL, jal);
    set_inst_func(JALR, jalr);
    set_inst_func(BEQ, beq);
    set_inst_func(BNE, bne);
    set_inst_func(BLT, blt);
    set_inst_func(BGE, bge);
    set_inst_func(BLTU, bltu);
    set_inst_func(BGEU, bgeu);
    set_inst_func(LB, lb);
    set_inst_func(LH, lh);
    set_inst_func(LW, lw);
    set_inst_func(LBU, lbu);
    set_inst_func(LHU, lhu);
    set_inst_func(SB, sb);
    set_inst_func(SH, sh);
    set_inst_func(SW, sw);
    set_inst_func(ADDI, addi);
    set_inst_func(SLTI, slti);
    set_inst_func(SLTIU, sltiu);
    set_inst_func(XORI, xori);
    set_inst_func(ORI, ori);
    set_inst_func(ANDI, andi);
    set_inst_func(SLLI, slli);
    set_inst_func(SRLI, srli);
    set_inst_func(SRAI, srai);
    set_inst_func(ADD, add);
    set_inst_func(SUB, sub);
    set_inst_func(SLL, sll);
    set_inst_func(SLT, slt);
    set_inst_func(SLTU, sltu);
    set_inst_func(XOR, xor);
    set_inst_func(SRL, srl);
    set_inst_func(SRA, sra);
    set_inst_func(OR, or);
    set_inst_func(AND, and);
    set_inst_func(FENCE, fence);
    set_inst_func(ECALL, ecall);
    set_inst_func(EBREAK, ebreak);
    set_inst_func(LWU, lwu);
    set_inst_func(LD, ld);
    set_inst_func(SD, sd);
    set_inst_func(ADDIW, addiw);
    set_inst_func(SLLIW, slliw);
    set_inst_func(SRLIW, srliw);
    set_inst_func(SRAIW, sraiw);
    set_inst_func(ADDW, addw);
    set_inst_func(SUBW, subw);
    set_inst_func(SLLW, sllw);
    set_inst_func(SRLW, srlw);
    set_inst_func(SRAW, sraw);

    //Zicsr
    set_inst_func(CSRRW, csrrw);
    set_inst_func(CSRRS, csrrs);
    set_inst_func(CSRRC, csrrc);
    set_inst_func(CSRRWI, csrrwi);
    set_inst_func(CSRRSI, csrrsi);
    set_inst_func(CSRRCI, csrrci);

    //mret & sret
    set_inst_func(MRET, mret);
    set_inst_func(SRET, sret);
}