#pragma once

#include <list>
#include <vector>
#include <string>
#include <stdint.h>
#include <ostream>
#include "namespace.h"
#include "vm_api.h"

VM_BEGIN

#define SUPEROPERATOR_START 100

class vmcode
  {
  public:    

    enum operation
      {
      ADD,
      ADDSD,      
      AND,
      CALL,
      CALLEXTERNAL,
      CMP,
      CMPEQPD,
      CMPLTPD,
      CMPLEPD,
      COMMENT,
      CQO,      
      CVTSI2SD,      
      CVTTSD2SI,
      CSQRT,
      CSIN,
      CCOS,
      CEXP,
      CLOG,
      CLOG2,
      CABS,
      CTAN,
      CATAN,
      CATAN2,
      CPOW,
      SWAP,
      DEC,
      DIV,
      DIV2, // same as DIV but without remainder to rdx
      DIVSD,  
      EXTERN,      
      GLOBAL,
      IDIV,
      IDIV2, // same as IDIV, but without remainder to rdx
      IMUL,
      INC,
      JMP,
      JA,
      JB,
      JE,
      JL,
      JLE,
      JG,
      JGE,
      JNE,
      JMPS, // JMP short
      JAS, // JA short
      JBS, // JB short
      JES, // JE short
      JLS, // JL short
      JLES, // JLE short
      JGS, // JG short
      JGES, // JGE short
      JNES, // JNE short      
      LABEL,
      LABEL_ALIGNED,
      MOD,
      MODSD,
      MOV,         
      MOVMSKPD,               
      MUL,
      MULSD,      
      NEG,
      NOP,
      OR,
      POP,
      PUSH,
      RET,
      SAL,
      SAR,      
      SETE,
      SETNE,
      SETL,
      SETG,
      SETLE,
      SETGE,
      SHL,      
      SHR,
      SQRTPD,      
      SUB,
      SUBSD,      
      TEST,
      UCOMISD,      
      XOR,
      XORPD,
      // super operators with three operands
      MOVADD = SUPEROPERATOR_START,
      MOVSUB,
      MOVMUL,
      MOVDIV,
      MOVSHL,
      MOVSHR,
      ADDSHL
      };

    enum operand
      {
      EMPTY,  
      RAX,
      RBX,
      RCX,
      RDX,
      RSI,
      RDI,
      RSP,
      RBP,
      R8,
      R9,
      R10,
      R11,
      R12,
      R13,
      R14,
      R15,  
#ifdef EXTRA_REGISTERS
      R16,
      R17,
      R18,
      R19,
      R20,
      R21,
      R22,
      R23,
      R24,
      R25,
      R26,
      R27,
      R28,
      R29,
      R30,
      R31,
#endif
      MEM_RAX,
      MEM_RBX,
      MEM_RCX,
      MEM_RDX,
      MEM_RDI,
      MEM_RSI,
      MEM_RSP,
      MEM_RBP,
      MEM_R8,
      MEM_R9,
      MEM_R10,
      MEM_R11,
      MEM_R12,
      MEM_R13,
      MEM_R14,
      MEM_R15,
#ifdef EXTRA_REGISTERS
      MEM_R16,
      MEM_R17,
      MEM_R18,
      MEM_R19,
      MEM_R20,
      MEM_R21,
      MEM_R22,
      MEM_R23,
      MEM_R24,
      MEM_R25,
      MEM_R26,
      MEM_R27,
      MEM_R28,
      MEM_R29,
      MEM_R30,
      MEM_R31,
#endif
      NUMBER,   
      LABELADDRESS,
      XMM0,
      XMM1,
      XMM2,
      XMM3,
      XMM4,
      XMM5,
      XMM6,
      XMM7,
      XMM8,
      XMM9,
      XMM10,
      XMM11,
      XMM12,
      XMM13,
      XMM14,
      XMM15,
#ifdef EXTRA_REGISTERS
      XMM16,
      XMM17,
      XMM18,
      XMM19,
      XMM20,
      XMM21,
      XMM22,
      XMM23,
      XMM24,
      XMM25,
      XMM26,
      XMM27,
      XMM28,
      XMM29,
      XMM30,
      XMM31
#endif
      };   

    struct instruction
      {
      operation oper;
      operand operand1, operand2, operand3;
      uint64_t operand1_mem, operand2_mem, operand3_mem;
      std::string text;

      VM_API instruction();
      VM_API instruction(const std::string& text);
      VM_API instruction(operation op);
      VM_API instruction(operation op, operand op1);
      VM_API instruction(operation op, operand op1, operand op2);
      VM_API instruction(operation op, operand op1, operand op2, operand op3);
      VM_API instruction(operation op, operand op1, uint64_t op1_mem);
      VM_API instruction(operation op, operand op1, uint64_t op1_mem, operand op2);
      VM_API instruction(operation op, operand op1, uint64_t op1_mem, operand op2, uint64_t op2_mem);
      VM_API instruction(operation op, operand op1, operand op2, uint64_t op2_mem);      
      VM_API instruction(operation op, const std::string& text);
      VM_API instruction(operation op, operand op1, const std::string& text);
      VM_API instruction(operation op, operand op1, operand op2, const std::string& text);
      VM_API instruction(operation op, operand op1, uint64_t op1_mem, const std::string& text);
      VM_API instruction(operation op, operand op1, uint64_t op1_mem, operand op2, const std::string& text);
      VM_API instruction(operation op, operand op1, uint64_t op1_mem, operand op2, uint64_t op2_mem, const std::string& text);
      VM_API instruction(operation op, operand op1, operand op2, uint64_t op2_mem, const std::string& text);

      VM_API void stream(std::ostream& out) const;
      //VM_API uint64_t fill_opcode(uint8_t* opcode_stream) const;
      };

    VM_API vmcode();
    VM_API ~vmcode();

    template <class... T>
    void add(T&&... val)
      {
      instruction ins(std::forward<T>(val)...);
      (*instructions_list_stack.back()).push_back(ins);
      }

    VM_API void push();
    VM_API void pop();

    VM_API const std::list<std::vector<instruction>>& get_instructions_list() const;
    VM_API std::list<std::vector<instruction>>& get_instructions_list();

    VM_API void stream(std::ostream& out) const;

    VM_API void clear();

    VM_API static std::string operation_to_string(operation op);
    VM_API static std::string operand_to_string(operand op);

  private:
    std::list<std::vector<instruction>> instructions_list;
    std::vector<std::list<std::vector<instruction>>::iterator> instructions_list_stack;


  };

VM_END
