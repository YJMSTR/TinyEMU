#include "../includes/monitor.h"
#include "../includes/common.h"
#include "../includes/cpu.h"
#include "../includes/mem.h"
char opt[MAX_OPTION_LENGTH];
char img_path[MAX_IMG_PATH_LENGTH];
void cmd_c() {
    cpu_exec(&cpu, -1);
}
void cmd_h() {
    puts("q: quit");
    puts("c: run");
    puts("r path: read .bin file from path to memory");
    puts("s: exec once");
    puts("h: help");
}
void cmd_r() {
    scanf("%s", img_path);
    FILE *fp = fopen(img_path, "rb");
    assert(fp);
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    printf("Read %ld byte from file.\n", size);
    fseek(fp, 0, SEEK_SET);
    fread(cpu.bus.dram.dram + RESET_VECTOR_OFFSET, size, 1, fp);
    fclose(fp);
}
void cmd_s() {
    exec_once(&cpu);
}
void cmd_re() {
    for (int i = 0; i < 32; i++) {
        printf("reg #%d == 0x%08lx\n", i, cpu.regs[i]);
    }
}