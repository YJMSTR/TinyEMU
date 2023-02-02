#include "includes/monitor.h"
#include "includes/cpu.h"
#include "includes/mem.h"

int main(int argc, char *argv[]) {
    //todo: 这里要执行一些初始化操作
    //...
    dram_init();
    cpu_init(&cpu);
    init_inst_func();
    while (1) {
        scanf("%s", opt);
        switch (opt[0]) {
            case 'q':
                return 0;
            case 'c':
                cmd_c();
                break;
            case 'h':
                cmd_h();
                break;
            case 'r':
                if (opt[1] == 'e') cmd_re();
                else cmd_r();
                break;
            case 's':
                cmd_s();
                break;
            default:
                puts("invalid command");
                puts("input \'h\' for help");
                break;
        }
    }
}