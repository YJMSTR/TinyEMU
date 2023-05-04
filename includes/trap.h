#ifndef __TRAP_H__
#define __TRAP_H__

#include "common.h"
#include "cpu.h"

enum TRAP {
    Contained,
    Requested,
    Invisible,
    Fatal
};

enum EXCEPTION {
    Instruction_address_misaligned = 0,
    Instruction_access_fault,
    Illegal_instruction,
    Breakpoint,
    Load_address_misaligned,
    Load_access_fault,
    Store_AMO_address_misaligned,
    Store_AMO_access_fault,
    Environment_call_from_U_mode,
    Environment_call_from_S_mode,
    Environment_call_from_M_mode = 11,
    Instruction_page_fault,
    Load_page_fault,
    Store_AMO_page_fault = 15 
};

void trap_handler(DECODER *decoder, enum TRAP traptype, bool isException, uint64_t cause, uint64_t tval);
#endif