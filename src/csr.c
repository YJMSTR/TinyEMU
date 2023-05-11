#include "../includes/csr.h"
#include "../includes/cpu.h"

void set_xpp(int cur_lev, int new_xpp) {
    switch (cur_lev)
    {
    case S:
        // clear spp
        cpu.csr.csr[sstatus] &= (-1) ^ (1 << 8);    
        // set spp
        cpu.csr.csr[sstatus] |= (new_xpp << 8);
        break;
    case M:
        // clear mpp
        cpu.csr.csr[mstatus] &= (-1) ^ (0b11 << 11);
        // set mpp 
        cpu.csr.csr[mstatus] |= (new_xpp << 11);
        break;
    default:
        cpu.state = CPU_STOP;
        puts("err:U-level call set_xpp\n");
        break;
    }
}

uint64_t get_xpp(int cur_lev) {
    switch (cur_lev)
    {
    case S:
        return (get_csr(sstatus) >> 8) & 1;
    case M:
        return (get_csr(mstatus) >> 11) & 0b11;
    default:
        cpu.state = CPU_STOP;
        puts("err:U-level call get_xpp\n");
        return -1;
    }
}

void set_xpie(int cur_lev, int new_xpie) {
    switch (cur_lev)
    {
    case S:
        // clear spie
        cpu.csr.csr[sstatus] &= (-1) ^ (1 << 5);
        // set spie
        cpu.csr.csr[sstatus] |= (new_xpie << 5);
        break;
    case M:
        // clear mpie
        cpu.csr.csr[mstatus] &= (-1) ^ (1 << 7);
        // set mpie
        cpu.csr.csr[mstatus] |= (new_xpie << 7); 
    default:
        cpu.state = CPU_STOP;
        puts("err:U-level call set_xpie");
        break;
    }
}

uint64_t get_xpie(int cur_lev) {
    switch (cur_lev)
    {
    case S:
        return (get_csr(sstatus) >> 5) & 1;
    case M:
        return (get_csr(mstatus) >> 7) & 1;
    default:
        cpu.state = CPU_STOP;
        puts("err:U-level call get_xpie");
        return -1;
    }
}

void set_xie(int cur_lev, int new_xie) {
    switch (cur_lev)
    {
    case S:
        // clear sie
        cpu.csr.csr[sstatus] &= (-1) ^ (1 << 1);
        // set sie
        cpu.csr.csr[sstatus] |= (new_xie << 1);
        break;
    case M:
        // clear mie
        cpu.csr.csr[mstatus] &= (-1) ^ (1 << 3);
        // set mie
        cpu.csr.csr[mstatus] |= (new_xie << 3); 
    default:
        cpu.state = CPU_STOP;
        puts("err:U-level call set_xie");
        break;
    }    
}

uint64_t get_xie(int cur_lev) {
    switch (cur_lev)
    {
    case S:
        return BITS(get_csr(sstatus), 1, 1);
    case M: 
        return BITS(get_csr(mstatus), 3, 3);
    default:
        cpu.state = CPU_STOP;
        puts("err:U-level call get_xie");
        return -1;
        break;
    }
}

void set_csr(uint64_t csr_addr, uint64_t csr_val) {
    cpu.csr.csr[csr_addr] = csr_val;
}

uint64_t get_csr(uint64_t csr_addr) {
    return cpu.csr.csr[csr_addr];
}