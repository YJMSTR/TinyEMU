#include "../includes/mem.h"
#include "../includes/common.h"

void dram_init(DRAM *dram) {
    dram->dram = malloc(DRAM_SIZE);
    assert(dram->dram);
}
//dram_store(0, 4, 0x00000297);//auipc t0, 0
void dram_store(DRAM *dram, uint64_t addr, int length, uint64_t val) {
    printf("dram store 0x%lx\n", addr);
    assert (length == 1 || length == 2 || length == 4 || length == 8);
    assert(addr >= 0 && addr < DRAM_SIZE);
    switch (length) {
        case 1:
            dram->dram[addr] = val & 0xff;
            return;
        case 2:
            dram->dram[addr] = val & 0xff;
            dram->dram[addr + 1] = (val >> 8) & 0xff;
            return;
        case 4:
            dram->dram[addr] = val & 0xff;
            dram->dram[addr + 1] = (val >> 8) & 0xff;
            dram->dram[addr + 2] = (val >> 16) & 0xff;
            dram->dram[addr + 3] = (val >> 24) & 0xff;
            return;
        case 8:
            printf("addr+4:0x%llx   addr: 0x%llx\n", addr + 4, addr);
            dram_store(dram, addr, 4, val & 0xffffffff);
            dram_store(dram, addr + 4, 4, (val >> 32) & 0xffffffff);
            return;
    }
}

uint64_t dram_load(DRAM *dram, uint64_t addr, int length) {
    assert (length == 1 || length == 2 || length == 4 || length == 8);
    assert(addr >= 0 && addr < DRAM_SIZE);
    switch (length) {
        case 1:
            return dram->dram[addr];
        case 2:
            return (((uint64_t)dram->dram[addr + 1]) << 8) | (uint64_t)dram->dram[addr];
        case 4:
            return (((uint64_t)dram->dram[addr + 3]) << 24) | ((uint64_t)dram->dram[addr + 2] << 16) | ((uint64_t)dram->dram[addr + 1] << 8) | ((uint64_t)dram->dram[addr]);
        case 8:
            printf("addr+4:0x%llx   addr: 0x%llx\n", dram_load(dram, addr + 4, 4), dram_load(dram, addr, 4));
            return (dram_load(dram, addr + 4, 4) << 32) | dram_load(dram, addr, 4);
    }
    return 0;
}

uint64_t mem_load(DRAM *dram, uint64_t addr, int length) {
    uint64_t res = dram_load(dram, addr - DRAM_BASE, length);
    printf("mem load addr = 0x%08lx, res = 0x%llx\n", addr, res);
    return res;
}

void mem_store(DRAM *dram, uint64_t addr, int length, uint64_t val) {
    dram_store(dram, addr - DRAM_BASE, length, val);
}
