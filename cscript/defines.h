#pragma once

#define variable_tag 1
#define virtual_machine_rsp_offset 8  // we need at least an offset of 8 because the virtual byte machine puts 0xffffffffffffffff as return value on the top of the stack 
#define rsp_offset 0

#define FIRST_TEMP_INTEGER_REG VM::vmcode::RAX
#define SECOND_TEMP_INTEGER_REG VM::vmcode::RBX

#define STACK_BACKUP_REGISTER VM::vmcode::R10
#define STACK_MEM_BACKUP_REGISTER VM::vmcode::MEM_R10

#define FIRST_TEMP_REAL_REG VM::vmcode::XMM4
#define SECOND_TEMP_REAL_REG VM::vmcode::XMM5

#define RESERVED_INTEGER_REG VM::vmcode::R11

#define RESERVED_REAL_REG VM::vmcode::XMM6
#define RESERVED_REAL_REG_2 VM::vmcode::XMM7

#define CALLING_CONVENTION_INT_PAR_1 VM::vmcode::RCX
#define CALLING_CONVENTION_INT_PAR_2 VM::vmcode::RDX
#define CALLING_CONVENTION_INT_PAR_3 VM::vmcode::R8
#define CALLING_CONVENTION_INT_PAR_4 VM::vmcode::R9

#define CALLING_CONVENTION_REAL_PAR_1 VM::vmcode::XMM0
#define CALLING_CONVENTION_REAL_PAR_2 VM::vmcode::XMM1
#define CALLING_CONVENTION_REAL_PAR_3 VM::vmcode::XMM2
#define CALLING_CONVENTION_REAL_PAR_4 VM::vmcode::XMM3