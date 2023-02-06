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
            dram->dram[addr + 1] = (val & 0xff00) >> 8;
            return;
        case 4:
            dram->dram[addr] = val & 0xff;
            dram->dram[addr + 1] = (val & 0xff00) >> 8;
            dram->dram[addr + 2] = (val & 0xff0000) >> 16;
            dram->dram[addr + 3] = (val & 0xff000000) >> 24;
            return;
        case 8:
            dram_store(dram, addr, 4, val & 0xffff);
            dram_store(dram, addr + 4, 4, (val & 0xffff0000) >> 32);
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
            return (dram->dram[addr + 1] << 8) | dram->dram[addr];
        case 4:
            return (dram->dram[addr + 3] << 24) | (dram->dram[addr + 2] << 16) | (dram->dram[addr + 1] << 8) | (dram->dram[addr]);
        case 8:
            return (dram_load(dram, addr + 4, 4) << 32) | dram_load(dram, addr, 4);
    }
    return 0;
}

uint64_t mem_load(DRAM *dram, uint64_t addr, int length) {
    printf("mem load addr = 0x%08lx\n", addr);
    return dram_load(dram, addr - DRAM_BASE, length);
}

void mem_store(DRAM *dram, uint64_t addr, int length, uint64_t val) {
    dram_store(dram, addr - DRAM_BASE, length, val);
}
