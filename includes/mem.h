#ifndef __MEM_H__
#define __MEM_H__
#include "common.h"

#define DRAM_SIZE 1024*1024*128ull   //128 MiB
#define DRAM_BASE 0x80000000ull
#ifndef RESET_VECTOR_OFFSET
#define RESET_VECTOR_OFFSET 0
#endif
#define RESET_VECTOR DRAM_BASE + RESET_VECTOR_OFFSET

extern uint8_t *dram;


void dram_init();
void dram_write(uint64_t addr, int length, uint64_t val);
uint64_t dram_read(uint64_t addr, int length);
uint64_t mem_read(uint64_t addr, int length);
void mem_write(uint64_t addr, int length, uint64_t val);
#endif