#ifndef __MEM_H__
#define __MEM_H__
#include "common.h"

#define DRAM_SIZE 1024*1024*128ull   //128 MiB
#define DRAM_BASE 0x80000000ull
#ifndef RESET_VECTOR_OFFSET
#define RESET_VECTOR_OFFSET 0
#endif
#define RESET_VECTOR DRAM_BASE + RESET_VECTOR_OFFSET

typedef struct DRAM {
    uint8_t *dram;
} DRAM;


void dram_init(DRAM *dram);
void dram_store(DRAM *dram, uint64_t addr, int length, uint64_t val);
uint64_t dram_load(DRAM *dram, uint64_t addr, int length);
uint64_t mem_load(DRAM *dram, uint64_t addr, int length);
void mem_store(DRAM *dram, uint64_t addr, int length, uint64_t val);
#endif