#include "../includes/trap.h"
#include "../includes/cpu.h"
#include "../includes/csr.h"

void trap_handler(DECODER *decoder, enum TRAP traptype, bool isException, uint64_t cause, uint64_t tval) {
    if (traptype == Fatal) {
        cpu.state = CPU_STOP;
        return;
    }
    enum CPU_PRI_LEVEL nxt_level = M;
    if (cpu.pri_level == S) {
        if ((isException && (get_csr(medeleg) & (1 << cause)))
            || (!isException && (get_csr(mideleg) & (1 << cause)))) {   
            nxt_level = S;
        } 
    }
    if (nxt_level == S) {
        set_xpp(S, cpu.pri_level);
        set_xpie(S, get_xie(cpu.pri_level));
        set_xie(S, 0);
        set_csr(sepc, cpu.pc);
        set_csr(stval, tval);
        set_csr(scause, (((uint64_t)traptype) << 63) | cause);
        uint64_t tvec = get_csr(stvec);
        decoder->dnpc = (BITS(tvec, 63, 2) << 2) + (BITS(tvec, 1, 0) == 1 ? cause * 4 : 0);
    } else {
        set_xpp(M, cpu.pri_level);
        set_xpie(M, get_xie(cpu.pri_level));
        set_xie(M, 0);
        set_csr(mepc, cpu.pc);
        set_csr(mtval, tval);
        set_csr(mcause, (((uint64_t)traptype) << 63) | cause);
        uint64_t tvec = get_csr(mtvec);
        decoder->dnpc = (BITS(tvec, 63, 2) << 2) + (BITS(tvec, 1, 0) == 1 ? cause * 4 : 0);
    }
}