#pragma once

#include "namespace.h"
#include "vm_api.h"
#include "vmcode.h"
#include <map>

VM_BEGIN

struct first_pass_data
  {
  first_pass_data() : size(0), data_size(0) {}

  uint64_t size, data_size;
  std::map<std::string, uint64_t> label_to_address;
  std::map<std::string, uint64_t> external_to_address;
  };

VM_API void* vm_bytecode(uint64_t& size, first_pass_data& d, vmcode& code, const std::map<std::string, uint64_t>& externals);
VM_API void* vm_bytecode(uint64_t& size, first_pass_data& d, vmcode& code);
VM_API void* vm_bytecode(uint64_t& size, vmcode& code, const std::map<std::string, uint64_t>& externals);
VM_API void* vm_bytecode(uint64_t& size, vmcode& code);

VM_API void free_bytecode(void* f, uint64_t size);

VM_API uint64_t disassemble_bytecode(vmcode::operation& op,
                                        vmcode::operand& operand1,
                                        vmcode::operand& operand2,
                                        vmcode::operand& operand3,
                                        uint64_t& operand1_mem,
                                        uint64_t& operand2_mem,
                                        uint64_t& operand3_mem,
                                        const uint8_t* bytecode);
  
#define carry_flag 1
#define overflow_flag 2048
#define sign_flag 64
#define zero_flag 128

#define operand_has_8bit_mem 128

struct registers
  {
  VM_API registers();
  uint64_t stack[256];
  uint64_t eflags;
  uint64_t reserved;
  uint64_t rax;
  uint64_t rbx;
  uint64_t rcx;
  uint64_t rdx;
  uint64_t rsi;
  uint64_t rdi;
  uint64_t rsp;
  uint64_t rbp;
  uint64_t r8;
  uint64_t r9;
  uint64_t r10;
  uint64_t r11;
  uint64_t r12;
  uint64_t r13;
  uint64_t r14;
  uint64_t r15;
#ifdef EXTRA_REGISTERS
  uint64_t r16;
  uint64_t r17;
  uint64_t r18;
  uint64_t r19;
  uint64_t r20;
  uint64_t r21;
  uint64_t r22;
  uint64_t r23;
  uint64_t r24;
  uint64_t r25;
  uint64_t r26;
  uint64_t r27;
  uint64_t r28;
  uint64_t r29;
  uint64_t r30;
  uint64_t r31;  
#endif
  double xmm0;
  double xmm1;
  double xmm2;
  double xmm3;
  double xmm4;
  double xmm5;
  double xmm6;
  double xmm7;
  double xmm8;
  double xmm9;
  double xmm10;
  double xmm11;
  double xmm12;
  double xmm13;
  double xmm14;
  double xmm15;
#ifdef EXTRA_REGISTERS
  double xmm16;
  double xmm17;
  double xmm18;
  double xmm19;
  double xmm20;
  double xmm21;
  double xmm22;
  double xmm23;
  double xmm24;
  double xmm25;
  double xmm26;
  double xmm27;
  double xmm28;
  double xmm29;
  double xmm30;
  double xmm31;
#endif  
  };

struct external_function
  {
  enum argtype
    {
    T_BOOL,
    T_CHAR_POINTER,
    T_DOUBLE,
    T_INT64,
    T_VOID
    };
  std::string name;
  uint64_t address;
  std::vector<argtype> arguments;
  argtype return_type;
  };

VM_API void run_bytecode(const uint8_t* bytecode, uint64_t size, registers& regs, const std::vector<external_function>& externals = std::vector<external_function>());

VM_END