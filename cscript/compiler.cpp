#include "compiler.h"

#include "utility.h"
#include "regalloc.h"

#include <variant>
#include <functional>
#include <map>
#include <sstream>

#define variable_tag 1
#define virtual_machine_rsp_offset 8  // we need at least an offset of 8 because the virtual byte machine puts 0xffffffffffffffff as return value on the top of the stack 
#define rsp_offset 0

#define FIRST_FREE_INTEGER_REG VM::vmcode::RAX // must be rax
#define SECOND_FREE_INTEGER_REG VM::vmcode::RBX

#define STACK_BACKUP_REGISTER VM::vmcode::R10
#define STACK_MEM_BACKUP_REGISTER VM::vmcode::MEM_R10

#define FIRST_FREE_REAL_REG VM::vmcode::XMM4
#define SECOND_FREE_REAL_REG VM::vmcode::XMM5

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

COMPILER_BEGIN

namespace
  {

  std::vector<VM::vmcode::operand> get_registers_for_integer_variables()
    {
    std::vector<VM::vmcode::operand> reg;
    reg.push_back(CALLING_CONVENTION_INT_PAR_1);
    reg.push_back(CALLING_CONVENTION_INT_PAR_2);
    reg.push_back(CALLING_CONVENTION_INT_PAR_3);
    reg.push_back(CALLING_CONVENTION_INT_PAR_4);
    return reg;
    }

  std::vector<VM::vmcode::operand> get_registers_for_real_variables()
    {
    std::vector<VM::vmcode::operand> reg;
    reg.push_back(CALLING_CONVENTION_REAL_PAR_1);
    reg.push_back(CALLING_CONVENTION_REAL_PAR_2);
    reg.push_back(CALLING_CONVENTION_REAL_PAR_3);
    reg.push_back(CALLING_CONVENTION_REAL_PAR_4);
    return reg;
    }


  enum storage_type
    {
    constant,
    external
    };

  enum value_type
    {
    single,
    integer,
    single_array,
    integer_array,
    pointer_to_single,
    pointer_to_integer
    };

  enum address_type
    {
    register_address,
    memory_address
    };

  struct variable_type
    {
    int64_t address;
    storage_type st;
    value_type vt;
    address_type at;
    };

  inline variable_type make_variable(int64_t addr, storage_type st, value_type vt, address_type at)
    {
    variable_type ret;
    ret.address = addr;
    ret.st = st;
    ret.vt = vt;
    ret.at = at;
    return ret;
    }

  typedef std::map<std::string, variable_type> variable_map;

  struct compile_data
    {
    compile_data() : ra(get_registers_for_integer_variables(), get_registers_for_real_variables()),
      max_stack_index(0), stack_index(0), var_offset(0), label(0)
      {
      }

    regalloc ra;
    int64_t max_stack_index;
    int64_t stack_index;
    variable_map vars;
    int64_t var_offset;
    uint64_t label;
    };

  enum return_type
    {
    RT_INTEGER,
    RT_REAL
    };

  return_type rt;

  void compile_expression(VM::vmcode& code, compile_data& data, const Expression& expr);
  void compile_statement(VM::vmcode& code, compile_data& data, const Statement& stm);

  std::string label_to_string(uint64_t lab)
    {
    std::stringstream str;
    str << "L_" << lab;
    return str.str();
    }

  void throw_compile_error(int line_nr, const std::string& message)
    {
    if (line_nr <= 0)
      throw std::logic_error("compile error: " + message);
    std::stringstream str;
    str << line_nr;
    throw std::logic_error("compile error: line " + str.str() + ": " + message);
    }

  void throw_compile_error(const std::string& message)
    {
    throw_compile_error(-1, message);
    }

  void index_to_real_operand(VM::vmcode::operand& op, int64_t& offset, int64_t stack_index)
    {
    switch (stack_index)
      {
      case 0: op = FIRST_FREE_REAL_REG; offset = 0; break;
      case 1: op = SECOND_FREE_REAL_REG; offset = 0; break;
        //case 2: op = VM::vmcode::XMM2; offset = 0; break;
        //case 3: op = VM::vmcode::XMM3; offset = 0; break;
        //case 4: op = VM::vmcode::XMM4; offset = 0; break;
        //case 5: op = VM::vmcode::XMM5; offset = 0; break;
      default: op = VM::vmcode::MEM_RSP; offset = -(stack_index - 1) * 8; break;
      };
    }

  void index_to_integer_operand(VM::vmcode::operand& op, int64_t& offset, int64_t stack_index)
    {
    switch (stack_index)
      {
      case 0: op = FIRST_FREE_INTEGER_REG; offset = 0; break;
      case 1: op = SECOND_FREE_INTEGER_REG; offset = 0; break;
      default: op = VM::vmcode::MEM_RSP; offset = -(stack_index - 1) * 8; break;
      };
    }

  void update_data(compile_data& data)
    {
    data.max_stack_index = data.stack_index > data.max_stack_index ? data.stack_index : data.max_stack_index;
    }

  void convert_integer_to_real(VM::vmcode& code, int64_t stack_index)
    {
    VM::vmcode::operand int_op, real_op;
    int64_t int_offset, real_offset;
    index_to_integer_operand(int_op, int_offset, stack_index);
    index_to_real_operand(real_op, real_offset, stack_index);
    if (int_offset)
      {
      if (real_offset)
        {
        //code.add(VM::vmcode::CVTSI2SD, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, int_offset);
        //code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, real_offset, RESERVED_REAL_REG);
        code.add(VM::vmcode::CVTSI2SD, VM::vmcode::MEM_RSP, real_offset, VM::vmcode::MEM_RSP, int_offset);
        }
      else
        code.add(VM::vmcode::CVTSI2SD, real_op, VM::vmcode::MEM_RSP, int_offset);
      }
    else
      {
      assert(real_offset == 0);
      code.add(VM::vmcode::CVTSI2SD, real_op, int_op);
      }
    }


  void convert_real_to_integer(VM::vmcode& code, int64_t stack_index)
    {
    VM::vmcode::operand int_op, real_op;
    int64_t int_offset, real_offset;
    index_to_integer_operand(int_op, int_offset, stack_index);
    index_to_real_operand(real_op, real_offset, stack_index);
    if (real_offset)
      {
      assert(int_offset);
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, real_offset);
      code.add(VM::vmcode::CVTTSD2SI, RESERVED_INTEGER_REG, RESERVED_REAL_REG);
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, int_offset, RESERVED_INTEGER_REG);
      }
    else
      {
      if (int_offset)
        {
        code.add(VM::vmcode::CVTTSD2SI, RESERVED_INTEGER_REG, real_op);
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, int_offset, RESERVED_INTEGER_REG);
        }
      else
        code.add(VM::vmcode::CVTTSD2SI, int_op, real_op);
      }
    }


  void compile_eq(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_real_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_real_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, offset1);
      op1 = RESERVED_REAL_REG;
      }
    if (offset2)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, VM::vmcode::MEM_RSP, offset2);
      op2 = RESERVED_REAL_REG_2;
      }
    code.add(VM::vmcode::CMPEQPD, op1, op2);

    index_to_integer_operand(op2, offset2, data.stack_index - 1);

    if (offset2)
      {
      code.add(VM::vmcode::MOVMSKPD, RESERVED_INTEGER_REG, op1);
      code.add(VM::vmcode::AND, RESERVED_INTEGER_REG, VM::vmcode::NUMBER, 1);
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset2, RESERVED_INTEGER_REG);
      }
    else
      {
      code.add(VM::vmcode::MOVMSKPD, op2, op1);
      code.add(VM::vmcode::AND, op2, VM::vmcode::NUMBER, 1);
      }
    rt = RT_INTEGER;
    }


  void compile_eqi(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_integer_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_integer_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      //code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset1);
      //code.add(VM::vmcode::CMP, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset2);
      code.add(VM::vmcode::CMP, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      }
    else if (offset2)
      {
      code.add(VM::vmcode::CMP, op1, VM::vmcode::MEM_RSP, offset2);
      }
    else
      code.add(VM::vmcode::CMP, op1, op2);

    if (offset1)
      {
      code.add(VM::vmcode::SETE, RESERVED_INTEGER_REG);
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset1, RESERVED_INTEGER_REG);
      }
    else
      {
      code.add(VM::vmcode::SETE, op1);
      }
    rt = RT_INTEGER;
    }


  void compile_neq(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_real_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_real_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, offset1);
      op1 = RESERVED_REAL_REG;
      }
    if (offset2)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, VM::vmcode::MEM_RSP, offset2);
      op2 = RESERVED_REAL_REG_2;
      }
    code.add(VM::vmcode::CMPEQPD, op1, op2);

    index_to_integer_operand(op2, offset2, data.stack_index - 1);

    if (offset2)
      {
      code.add(VM::vmcode::MOVMSKPD, RESERVED_INTEGER_REG, op1);
      code.add(VM::vmcode::AND, RESERVED_INTEGER_REG, VM::vmcode::NUMBER, 1);
      code.add(VM::vmcode::XOR, RESERVED_INTEGER_REG, VM::vmcode::NUMBER, 1);
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset2, RESERVED_INTEGER_REG);
      }
    else
      {
      code.add(VM::vmcode::MOVMSKPD, op2, op1);
      code.add(VM::vmcode::AND, op2, VM::vmcode::NUMBER, 1);
      code.add(VM::vmcode::XOR, op2, VM::vmcode::NUMBER, 1);
      }
    rt = RT_INTEGER;
    }


  void compile_neqi(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_integer_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_integer_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      //code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset1);
      //code.add(VM::vmcode::CMP, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset2);
      code.add(VM::vmcode::CMP, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      }
    else if (offset2)
      {
      code.add(VM::vmcode::CMP, op1, VM::vmcode::MEM_RSP, offset2);
      }
    else
      code.add(VM::vmcode::CMP, op1, op2);

    if (offset1)
      {
      code.add(VM::vmcode::SETNE, RESERVED_INTEGER_REG);
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset1, RESERVED_INTEGER_REG);
      }
    else
      {
      code.add(VM::vmcode::SETNE, op1);
      }
    rt = RT_INTEGER;
    }

  void compile_less(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_real_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_real_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, offset1);
      op1 = RESERVED_REAL_REG;
      }
    if (offset2)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, VM::vmcode::MEM_RSP, offset2);
      op2 = RESERVED_REAL_REG_2;
      }
    code.add(VM::vmcode::CMPLTPD, op1, op2);

    index_to_integer_operand(op2, offset2, data.stack_index - 1);

    if (offset2)
      {
      code.add(VM::vmcode::MOVMSKPD, RESERVED_INTEGER_REG, op1);
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset2, RESERVED_INTEGER_REG);
      }
    else
      code.add(VM::vmcode::MOVMSKPD, op2, op1);

    rt = RT_INTEGER;
    }


  void compile_lessi(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_integer_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_integer_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      //code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset1);
      //code.add(VM::vmcode::CMP, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset2);
      code.add(VM::vmcode::CMP, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      }
    else if (offset2)
      {
      code.add(VM::vmcode::CMP, op1, VM::vmcode::MEM_RSP, offset2);
      }
    else
      code.add(VM::vmcode::CMP, op1, op2);

    if (offset1)
      {
      code.add(VM::vmcode::SETL, RESERVED_INTEGER_REG);
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset1, RESERVED_INTEGER_REG);
      }
    else
      {
      code.add(VM::vmcode::SETL, op1);
      }
    rt = RT_INTEGER;
    }

  void compile_leq(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_real_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_real_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, offset1);
      op1 = RESERVED_REAL_REG;
      }
    if (offset2)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, VM::vmcode::MEM_RSP, offset2);
      op2 = RESERVED_REAL_REG_2;
      }
    code.add(VM::vmcode::CMPLEPD, op1, op2);

    index_to_integer_operand(op2, offset2, data.stack_index - 1);

    if (offset2)
      {
      code.add(VM::vmcode::MOVMSKPD, RESERVED_INTEGER_REG, op1);
      code.add(VM::vmcode::AND, RESERVED_INTEGER_REG, VM::vmcode::NUMBER, 1);
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset2, RESERVED_INTEGER_REG);
      }
    else
      {
      code.add(VM::vmcode::MOVMSKPD, op2, op1);
      code.add(VM::vmcode::AND, op2, VM::vmcode::NUMBER, 1);
      }

    rt = RT_INTEGER;
    }


  void compile_leqi(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_integer_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_integer_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      //code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset1);
      //code.add(VM::vmcode::CMP, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset2);
      code.add(VM::vmcode::CMP, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      }
    else if (offset2)
      {
      code.add(VM::vmcode::CMP, op1, VM::vmcode::MEM_RSP, offset2);
      }
    else
      code.add(VM::vmcode::CMP, op1, op2);

    if (offset1)
      {
      code.add(VM::vmcode::SETLE, RESERVED_INTEGER_REG);     
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset1, RESERVED_INTEGER_REG);
      }
    else
      {
      code.add(VM::vmcode::SETLE, op1);
      }
    rt = RT_INTEGER;
    }


  void compile_greater(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_real_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_real_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, offset1);
      op1 = RESERVED_REAL_REG;
      }
    if (offset2)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, VM::vmcode::MEM_RSP, offset2);
      op2 = RESERVED_REAL_REG_2;
      }
    code.add(VM::vmcode::CMPLTPD, op2, op1);

    index_to_integer_operand(op1, offset1, data.stack_index - 1);

    if (offset1)
      {
      code.add(VM::vmcode::MOVMSKPD, RESERVED_INTEGER_REG, op2);
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset1, RESERVED_INTEGER_REG);
      }
    else
      {
      code.add(VM::vmcode::MOVMSKPD, op1, op2);

      }
    rt = RT_INTEGER;
    }


  void compile_greateri(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_integer_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_integer_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      //code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset1);
      //code.add(VM::vmcode::CMP, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset2);
      code.add(VM::vmcode::CMP, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      }
    else if (offset2)
      {
      code.add(VM::vmcode::CMP, op1, VM::vmcode::MEM_RSP, offset2);
      }
    else
      code.add(VM::vmcode::CMP, op1, op2);

    if (offset1)
      {
      code.add(VM::vmcode::SETG, RESERVED_INTEGER_REG);
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset1, RESERVED_INTEGER_REG);
      }
    else
      {
      code.add(VM::vmcode::SETG, op1);
      }
    rt = RT_INTEGER;
    }


  void compile_geq(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_real_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_real_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, offset1);
      op1 = RESERVED_REAL_REG;
      }
    if (offset2)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, VM::vmcode::MEM_RSP, offset2);
      op2 = RESERVED_REAL_REG_2;
      }
    code.add(VM::vmcode::CMPLEPD, op2, op1);

    index_to_integer_operand(op1, offset1, data.stack_index - 1);

    if (offset1)
      {
      code.add(VM::vmcode::MOVMSKPD, RESERVED_INTEGER_REG, op2);
      code.add(VM::vmcode::AND, RESERVED_INTEGER_REG, VM::vmcode::NUMBER, 1);
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset1, RESERVED_INTEGER_REG);
      }
    else
      {
      code.add(VM::vmcode::MOVMSKPD, op1, op2);
      code.add(VM::vmcode::AND, op1, VM::vmcode::NUMBER, 1);
      }
    rt = RT_INTEGER;
    }


  void compile_geqi(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_integer_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_integer_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      //code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset1);
      //code.add(VM::vmcode::CMP, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset2);
      code.add(VM::vmcode::CMP, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      }
    else if (offset2)
      {
      code.add(VM::vmcode::CMP, op1, VM::vmcode::MEM_RSP, offset2);
      }
    else
      code.add(VM::vmcode::CMP, op1, op2);

    if (offset1)
      {
      code.add(VM::vmcode::SETGE, RESERVED_INTEGER_REG);
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset1, RESERVED_INTEGER_REG);
      }
    else
      {
      code.add(VM::vmcode::SETGE, op1);
      }
    rt = RT_INTEGER;
    }

  void compile_add(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_real_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_real_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      if (offset2)
        code.add(VM::vmcode::ADDSD, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::ADDSD, VM::vmcode::MEM_RSP, offset1, op2);
      }
    else
      {
      if (offset2)
        code.add(VM::vmcode::ADDSD, op1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::ADDSD, op1, op2);
      }
    /*
    if (offset1)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, offset1);
      op1 = RESERVED_REAL_REG;
      }
    if (offset2)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, VM::vmcode::MEM_RSP, offset2);
      op2 = RESERVED_REAL_REG_2;
      }
    code.add(VM::vmcode::ADDSD, op1, op2);
    if (offset1)
      {
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset1, op1);
      }
    */
    rt = RT_REAL;
    }

  void compile_addi(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_integer_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_integer_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      if (offset2)
        code.add(VM::vmcode::ADD, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::ADD, VM::vmcode::MEM_RSP, offset1, op2);
      }
    else
      {
      if (offset2)
        code.add(VM::vmcode::ADD, op1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::ADD, op1, op2);
      }
    /*
    if (offset1)
      {
      code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset1);
      op1 = RESERVED_INTEGER_REG;
      }
    if (offset2)
      {
      code.add(VM::vmcode::ADD, op1, VM::vmcode::MEM_RSP, offset2);
      }
    else
      code.add(VM::vmcode::ADD, op1, op2);
    if (offset1)
      {
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset1, RESERVED_INTEGER_REG);
      }
    */
    rt = RT_INTEGER;
    }

  void compile_div(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_real_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_real_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      if (offset2)
        code.add(VM::vmcode::DIVSD, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::DIVSD, VM::vmcode::MEM_RSP, offset1, op2);
      }
    else
      {
      if (offset2)
        code.add(VM::vmcode::DIVSD, op1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::DIVSD, op1, op2);
      }
    /*
    if (offset1)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, offset1);
      op1 = RESERVED_REAL_REG;
      }
    if (offset2)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, VM::vmcode::MEM_RSP, offset2);
      op2 = RESERVED_REAL_REG_2;
      }
    code.add(VM::vmcode::DIVSD, op1, op2);
    if (offset1)
      {
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset1, op1);
      }
    */
    rt = RT_REAL;
    }


  void compile_divi(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_integer_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_integer_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      if (offset2)
        code.add(VM::vmcode::IDIV2, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::IDIV2, VM::vmcode::MEM_RSP, offset1, op2);
      }
    else
      {
      if (offset2)
        code.add(VM::vmcode::IDIV2, op1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::IDIV2, op1, op2);
      }
    /*
    if (op1 == VM::vmcode::RAX)
      {
      if (offset2)
        code.add(VM::vmcode::IDIV2, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::IDIV2, op2);
      }
    else
      {
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -data.stack_index * 8, VM::vmcode::RAX); // save RAX
      if (offset1)
        {
        code.add(VM::vmcode::MOV, VM::vmcode::RAX, VM::vmcode::MEM_RSP, offset1);
        }
      else
        code.add(VM::vmcode::MOV, VM::vmcode::RAX, op1);

      if (offset2)
        code.add(VM::vmcode::IDIV2, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::IDIV2, op2);

      if (offset1)
        {
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset1, VM::vmcode::RAX);
        }
      else
        code.add(VM::vmcode::MOV, op1, VM::vmcode::RAX);
      code.add(VM::vmcode::MOV, VM::vmcode::RAX, VM::vmcode::MEM_RSP, -data.stack_index * 8);
      }
      */
    rt = RT_INTEGER;
    }

  void compile_sub(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_real_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_real_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      if (offset2)
        code.add(VM::vmcode::SUBSD, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::SUBSD, VM::vmcode::MEM_RSP, offset1, op2);
      }
    else
      {
      if (offset2)
        code.add(VM::vmcode::SUBSD, op1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::SUBSD, op1, op2);
      }
    /*
    if (offset1)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, offset1);
      op1 = RESERVED_REAL_REG;
      }
    if (offset2)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, VM::vmcode::MEM_RSP, offset2);
      op2 = RESERVED_REAL_REG_2;
      }
    code.add(VM::vmcode::SUBSD, op1, op2);
    if (offset1)
      {
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset1, op1);
      }
    */
    rt = RT_REAL;
    }

  void compile_subi(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_integer_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_integer_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      if (offset2)
        code.add(VM::vmcode::SUB, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::SUB, VM::vmcode::MEM_RSP, offset1, op2);
      }
    else
      {
      if (offset2)
        code.add(VM::vmcode::SUB, op1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::SUB, op1, op2);
      }
    /*
    if (offset1)
      {
      code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset1);
      op1 = RESERVED_INTEGER_REG;
      }
    if (offset2)
      {
      code.add(VM::vmcode::SUB, op1, VM::vmcode::MEM_RSP, offset2);
      }
    else
      code.add(VM::vmcode::SUB, op1, op2);
    if (offset1)
      {
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset1, RESERVED_INTEGER_REG);
      }
    */
    rt = RT_INTEGER;
    }

  void compile_mul(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_real_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_real_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      if (offset2)
        code.add(VM::vmcode::MULSD, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::MULSD, VM::vmcode::MEM_RSP, offset1, op2);
      }
    else
      {
      if (offset2)
        code.add(VM::vmcode::MULSD, op1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::MULSD, op1, op2);
      }
    /*
    if (offset1)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, offset1);
      op1 = RESERVED_REAL_REG;
      }
    if (offset2)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, VM::vmcode::MEM_RSP, offset2);
      op2 = RESERVED_REAL_REG_2;
      }
    code.add(VM::vmcode::MULSD, op1, op2);
    if (offset1)
      {
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset1, op1);
      }
    */
    rt = RT_REAL;
    }

  void compile_muli(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_integer_operand(op1, offset1, data.stack_index - 1);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_integer_operand(op2, offset2, data.stack_index);
    if (offset1)
      {
      if (offset2)
        code.add(VM::vmcode::IMUL, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::IMUL, VM::vmcode::MEM_RSP, offset1, op2);
      }
    else
      {
      if (offset2)
        code.add(VM::vmcode::IMUL, op1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::IMUL, op1, op2);
      }
    /*
    if (offset1)
      {
      code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset1);
      op1 = RESERVED_INTEGER_REG;
      }
    if (offset2)
      {
      code.add(VM::vmcode::IMUL, op1, VM::vmcode::MEM_RSP, offset2);
      }
    else
      code.add(VM::vmcode::IMUL, op1, op2);
    if (offset1)
      {
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset1, RESERVED_INTEGER_REG);
      }
    */
    rt = RT_INTEGER;
    }

  void change_sign_real(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_real_operand(op1, offset1, data.stack_index);
    if (offset1)
      {
      code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, offset1);
      op1 = RESERVED_REAL_REG;
      }
    code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::NUMBER, 0x8000000000000000);
    code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, RESERVED_INTEGER_REG);
    code.add(VM::vmcode::XORPD, op1, RESERVED_REAL_REG_2);
    if (offset1)
      {
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset1, op1);
      }
    }

  void change_sign_integer(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_integer_operand(op1, offset1, data.stack_index);
    if (offset1)
      {
      //code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset1);
      //code.add(VM::vmcode::NEG, RESERVED_INTEGER_REG);
      //code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset1, RESERVED_INTEGER_REG);
      code.add(VM::vmcode::NEG, VM::vmcode::MEM_RSP, offset1);
      }
    else
      code.add(VM::vmcode::NEG, op1);
    }

  void compile_sqrt(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op;
    int64_t offset;
    index_to_real_operand(op, offset, data.stack_index);
    //if (offset)
    //  {
    //  code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, offset);
    //  op = RESERVED_REAL_REG;
    //  }
    if (offset)
      code.add(VM::vmcode::SQRTPD, VM::vmcode::MEM_RSP, offset, VM::vmcode::MEM_RSP, offset);
    else
      code.add(VM::vmcode::SQRTPD, op, op);
    //if (offset)
    //  {
    //  code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, RESERVED_REAL_REG);
    //  }
    }

  struct sin_functor
    {
    void operator () (VM::vmcode& code)
      {
      code.add(VM::vmcode::FSIN);
      }
    };

  struct cos_functor
    {
    void operator () (VM::vmcode& code)
      {
      code.add(VM::vmcode::FCOS);
      }
    };

  struct exp_functor
    {
    void operator () (VM::vmcode& code)
      {
      code.add(VM::vmcode::FLDL2E);
      code.add(VM::vmcode::FMULP, VM::vmcode::ST1, VM::vmcode::ST0);
      code.add(VM::vmcode::FLD1);
      code.add(VM::vmcode::FSCALE);
      code.add(VM::vmcode::FXCH);
      code.add(VM::vmcode::FLD1);
      code.add(VM::vmcode::FXCH);
      code.add(VM::vmcode::FPREM);
      code.add(VM::vmcode::F2XM1);
      code.add(VM::vmcode::FADDP, VM::vmcode::ST1, VM::vmcode::ST0);
      code.add(VM::vmcode::FMULP, VM::vmcode::ST1, VM::vmcode::ST0);
      }
    };

  struct log_functor
    {
    void operator () (VM::vmcode& code)
      {
      code.add(VM::vmcode::FLDLN2);
      code.add(VM::vmcode::FXCH);
      code.add(VM::vmcode::FYL2X);
      }
    };

  struct log2_functor
    {
    void operator () (VM::vmcode& code)
      {
      code.add(VM::vmcode::FLD1);
      code.add(VM::vmcode::FXCH);
      code.add(VM::vmcode::FYL2X);
      }
    };

  struct fabs_functor
    {
    void operator () (VM::vmcode& code)
      {
      code.add(VM::vmcode::FABS);
      }
    };

  template <class T>
  void compile_fpu_fun1(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op, op_dummy;
    int64_t offset;
    index_to_real_operand(op, offset, data.stack_index);
    if (offset)
      {
      code.add(VM::vmcode::FLD, VM::vmcode::MEM_RSP, offset);
      }
    else
      {
      index_to_integer_operand(op_dummy, offset, data.stack_index); // take integer operand for faster offset
      int j = 0;
      while (!offset)
        {
        ++j;
        index_to_integer_operand(op_dummy, offset, data.stack_index + j);
        }
      data.stack_index += j;
      update_data(data);
      data.stack_index -= j;
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, op);
      code.add(VM::vmcode::FLD, VM::vmcode::MEM_RSP, offset);
      }

    T()(code);

    code.add(VM::vmcode::FSTP, VM::vmcode::MEM_RSP, offset);

    if (op != VM::vmcode::MEM_RSP)
      {
      code.add(VM::vmcode::MOV, op, VM::vmcode::MEM_RSP, offset);
      }
    }

  typedef std::function<void(VM::vmcode&, compile_data&)> c_func;
  typedef std::map<std::string, c_func> c_funcs_t;


  c_funcs_t c_funcs =
    {
      {"+", compile_add},
      {"+i", compile_addi},
      {"-", compile_sub},
      {"-i", compile_subi},
      {"*", compile_mul},
      {"*i", compile_muli},
      {"/", compile_div},
      {"/i", compile_divi},
      {"<", compile_less},
      {"<i", compile_lessi},
      {"<=", compile_leq},
      {"<=i", compile_leqi},
      {">", compile_greater},
      {">i", compile_greateri},
      {">=", compile_geq},
      {">=i", compile_geqi},
      {"==", compile_eq},
      {"==i", compile_eqi},
      {"!=", compile_neq},
      {"!=i", compile_neqi},
      {"sqrt", compile_sqrt},
      {"sin", compile_fpu_fun1<sin_functor>},
      {"cos", compile_fpu_fun1<cos_functor>},
      {"exp", compile_fpu_fun1<exp_functor>},
      {"log", compile_fpu_fun1<log_functor>},
      {"log2", compile_fpu_fun1<log2_functor>},
      {"fabs", compile_fpu_fun1<fabs_functor>}
    };

  void compile_make_int_array(VM::vmcode& code, compile_data& data, const Int& make_i)
    {
    if (make_i.dims.size() != 1)
      throw_compile_error(make_i.line_nr, "compile error: only single dimension arrays are allowed");
    if (!make_i.expr.operands.empty())
      throw_compile_error(make_i.line_nr, "compile error: array initialization is not allowed");
    if (!is_constant(make_i.dims.front()))
      throw_compile_error(make_i.line_nr, "compile error: array dimension should be a constant");
    int64_t val = to_i(get_constant_value(make_i.dims.front()));
    if (val <= 0)
      throw_compile_error(make_i.line_nr, "compile error: array dimension should be a non zero positive number");
    auto it = data.vars.find(make_i.name);
    if (it == data.vars.end())
      {
      int64_t var_id = data.var_offset | variable_tag;
      data.vars.insert(std::make_pair(make_i.name, make_variable(var_id, constant, integer_array, memory_address)));
      data.var_offset += 8 * val;
      }
    else
      {
      throw_compile_error(make_i.line_nr, "Variable " + make_i.name + " already exists");
      }
    }

  void compile_make_int_single(VM::vmcode& code, compile_data& data, const Int& make_i)
    {
    bool init = !make_i.expr.operands.empty();
    VM::vmcode::operand int_op;
    if (init)
      {
      compile_expression(code, data, make_i.expr);
      if (rt == RT_REAL)
        {
        convert_real_to_integer(code, data.stack_index);
        }
      rt = RT_INTEGER;
      int64_t int_offset;
      index_to_integer_operand(int_op, int_offset, data.stack_index);
      if (int_offset)
        {
        code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, int_offset);
        int_op = RESERVED_INTEGER_REG;
        }
      }
    auto it = data.vars.find(make_i.name);
    if (it == data.vars.end())
      {
      if (data.ra.free_integer_register_available())
        {
        auto reg = data.ra.get_next_available_integer_register();
        data.ra.make_integer_register_unavailable(reg);
        if (init)
          code.add(VM::vmcode::MOV, reg, int_op);
        data.vars.insert(std::make_pair(make_i.name, make_variable(reg, constant, integer, register_address)));
        }
      else
        {
        int64_t var_id = data.var_offset | variable_tag;
        if (init)
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, int_op);
        data.vars.insert(std::make_pair(make_i.name, make_variable(var_id, constant, integer, memory_address)));
        data.var_offset += 8;
        }
      }
    else
      {
      throw_compile_error(make_i.line_nr, "Variable " + make_i.name + " already exists");
      }
    }

  void compile_make_int(VM::vmcode& code, compile_data& data, const Int& make_i)
    {
    if (!make_i.dims.empty())
      compile_make_int_array(code, data, make_i);
    else
      compile_make_int_single(code, data, make_i);
    }


  void compile_make_float_array(VM::vmcode& code, compile_data& data, const Float& make_f)
    {
    if (make_f.dims.size() != 1)
      throw_compile_error(make_f.line_nr, "compile error: only single dimension arrays are allowed");
    if (!make_f.expr.operands.empty())
      throw_compile_error(make_f.line_nr, "compile error: array initialization is not allowed");
    if (!is_constant(make_f.dims.front()))
      throw_compile_error(make_f.line_nr, "compile error: array dimension should be a constant");
    int64_t val = to_i(get_constant_value(make_f.dims.front()));
    if (val <= 0)
      throw_compile_error(make_f.line_nr, "compile error: array dimension should be a non zero positive number");
    auto it = data.vars.find(make_f.name);
    if (it == data.vars.end())
      {
      int64_t var_id = data.var_offset | variable_tag;
      data.vars.insert(std::make_pair(make_f.name, make_variable(var_id, constant, single_array, memory_address)));
      data.var_offset += 8 * val;
      }
    else
      {
      throw_compile_error(make_f.line_nr, "Variable " + make_f.name + " already exists");
      }
    }

  void compile_make_float_single(VM::vmcode& code, compile_data& data, const Float& make_f)
    {
    bool init = !make_f.expr.operands.empty();
    VM::vmcode::operand float_op;
    int64_t float_offset;
    if (init)
      {
      compile_expression(code, data, make_f.expr);
      if (rt == RT_INTEGER)
        {
        convert_integer_to_real(code, data.stack_index);
        }

      rt = RT_REAL;
      index_to_real_operand(float_op, float_offset, data.stack_index);
      if (float_offset)
        {
        code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, float_offset);
        }
      }
    auto it = data.vars.find(make_f.name);
    if (it == data.vars.end())
      {
      if (data.ra.free_real_register_available())
        {
        auto reg = data.ra.get_next_available_real_register();
        data.ra.make_real_register_unavailable(reg);
        if (init)
          {
          if (float_offset)
            code.add(VM::vmcode::MOV, reg, RESERVED_INTEGER_REG);
          else
            code.add(VM::vmcode::MOV, reg, float_op);
          }
        data.vars.insert(std::make_pair(make_f.name, make_variable(reg, constant, single, register_address)));
        }
      else
        {
        int64_t var_id = data.var_offset | variable_tag;
        if (init)
          {
          if (float_offset)
            code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, RESERVED_INTEGER_REG);
          else
            code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, float_op);
          }
        data.vars.insert(std::make_pair(make_f.name, make_variable(var_id, constant, single, memory_address)));
        data.var_offset += 8;
        }
      }
    else
      {
      throw_compile_error(make_f.line_nr, "Variable " + make_f.name + " already exists");
      }
    }

  void compile_make_float(VM::vmcode& code, compile_data& data, const Float& make_f)
    {
    if (!make_f.dims.empty())
      compile_make_float_array(code, data, make_f);
    else
      compile_make_float_single(code, data, make_f);
    }

  void compile_variable(VM::vmcode& code, compile_data& data, const Variable& v)
    {
    auto it = data.vars.find(v.name);
    if (it == data.vars.end())
      throw_compile_error(v.line_nr, "Cannot find variable " + v.name);
    int64_t var_id = it->second.address;
    rt = (it->second.vt == single) ? RT_REAL : RT_INTEGER;
    if (it->second.st == constant)
      {
      if (rt == RT_INTEGER)
        {
        VM::vmcode::operand int_op;
        int64_t int_offset;
        index_to_integer_operand(int_op, int_offset, data.stack_index);
        if (int_offset)
          {
          if (it->second.at == memory_address)
            {
            code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, -var_id);
            code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, int_offset, RESERVED_INTEGER_REG);
            }
          else
            {
            code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, int_offset, (VM::vmcode::operand)var_id);
            }
          }
        else
          {
          if (it->second.at == memory_address)
            {
            code.add(VM::vmcode::MOV, int_op, VM::vmcode::MEM_RSP, -var_id);
            }
          else
            {
            code.add(VM::vmcode::MOV, int_op, (VM::vmcode::operand)var_id);
            }
          }
        }
      else
        {
        VM::vmcode::operand real_op;
        int64_t real_offset;
        index_to_real_operand(real_op, real_offset, data.stack_index);
        if (real_offset)
          {
          if (it->second.at == memory_address)
            {
            code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, -var_id);
            code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, real_offset, RESERVED_INTEGER_REG);
            }
          else
            {
            code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, real_offset, (VM::vmcode::operand)var_id);
            }
          }
        else
          {
          if (it->second.at == memory_address)
            {
            code.add(VM::vmcode::MOV, real_op, VM::vmcode::MEM_RSP, -var_id);
            }
          else
            {
            code.add(VM::vmcode::MOV, real_op, (VM::vmcode::operand)var_id);
            }
          }
        }
      }
    else
      {
      throw std::runtime_error("not implemented");
      }
    }

  void compile_assignment_pointer(VM::vmcode& code, compile_data& data, const Assignment& a)
    {
    if (a.dims.size() != 1)
      throw_compile_error(a.line_nr, "only single dimension arrays are allowed");
    if (data.stack_index != 0)
      throw_compile_error(a.line_nr, "assignment only as single statement allowed");
    compile_expression(code, data, a.dims.front());
    if (rt == RT_REAL)
      {
      convert_real_to_integer(code, data.stack_index);
      rt = RT_INTEGER;
      }
    code.add(VM::vmcode::SHL, FIRST_FREE_INTEGER_REG, VM::vmcode::NUMBER, 3);
    ++data.stack_index;
    update_data(data);
    compile_expression(code, data, a.expr);
    auto it = data.vars.find(a.name);
    if (it == data.vars.end())
      throw_compile_error(a.line_nr, "variable " + a.name + " is not declared.");
    //if (it->second.st != constant)
    //  throw_compile_error(a.line_nr, "variable " + a.name + " cannot be assigned.");
    int64_t var_id = it->second.address;
    if (it->second.vt != pointer_to_single && it->second.vt != pointer_to_integer)
      throw_compile_error(a.line_nr, "I expect " + a.name + " to be a pointer.");
    if (it->second.vt == pointer_to_single && rt == RT_INTEGER)
      {
      convert_integer_to_real(code, data.stack_index);
      rt = RT_REAL;
      }
    else if (it->second.vt == pointer_to_integer && rt == RT_REAL)
      {
      convert_real_to_integer(code, data.stack_index);
      rt = RT_INTEGER;
      }
    --data.stack_index;

    if (it->second.at == memory_address)
      code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::MEM_RSP, -var_id);
    else
      code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, (VM::vmcode::operand)var_id);
    code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, FIRST_FREE_INTEGER_REG);
    //r10 contains the pointer address
    //SECOND_FREE_REAL_REG or SECOND_FREE_INTEGER_REG contains the expression value
    switch (a.op.front())
      {
      case '=':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, SECOND_FREE_REAL_REG);
        }
      else
        {
        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, SECOND_FREE_INTEGER_REG);
        }
      break;
      }
      case '+':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::MOV, RESERVED_REAL_REG, STACK_MEM_BACKUP_REGISTER);
        code.add(VM::vmcode::ADDSD, RESERVED_REAL_REG, SECOND_FREE_REAL_REG);
        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, RESERVED_REAL_REG);
        }
      else
        {
        code.add(VM::vmcode::ADD, STACK_MEM_BACKUP_REGISTER, SECOND_FREE_INTEGER_REG);
        }
      break;
      }
      case '-':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::MOV, RESERVED_REAL_REG, STACK_MEM_BACKUP_REGISTER);
        code.add(VM::vmcode::SUBSD, RESERVED_REAL_REG, SECOND_FREE_REAL_REG);
        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, RESERVED_REAL_REG);
        }
      else
        {
        code.add(VM::vmcode::SUB, STACK_MEM_BACKUP_REGISTER, SECOND_FREE_INTEGER_REG);
        }
      break;
      }
      case '*':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::MOV, RESERVED_REAL_REG, STACK_MEM_BACKUP_REGISTER);
        code.add(VM::vmcode::MULSD, RESERVED_REAL_REG, SECOND_FREE_REAL_REG);
        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, RESERVED_REAL_REG);
        }
      else
        {
        code.add(VM::vmcode::IMUL, SECOND_FREE_INTEGER_REG, STACK_MEM_BACKUP_REGISTER);
        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, SECOND_FREE_INTEGER_REG);
        }
      break;
      }
      case '/':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::MOV, RESERVED_REAL_REG, STACK_MEM_BACKUP_REGISTER);
        code.add(VM::vmcode::DIVSD, RESERVED_REAL_REG, SECOND_FREE_REAL_REG);
        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, RESERVED_REAL_REG);
        }
      else
        {
        //code.add(VM::vmcode::MOV, VM::vmcode::RAX, STACK_MEM_BACKUP_REGISTER);
        code.add(VM::vmcode::IDIV2, STACK_MEM_BACKUP_REGISTER, SECOND_FREE_INTEGER_REG);
        //code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, VM::vmcode::RAX);
        }
      break;
      }
      default: throw std::runtime_error("not implemented");
      };
    }

  void compile_assignment_array(VM::vmcode& code, compile_data& data, const Assignment& a)
    {
    if (a.dims.size() != 1)
      throw_compile_error(a.line_nr, "only single dimension arrays are allowed");
    if (data.stack_index != 0)
      throw_compile_error(a.line_nr, "assignment only as single statement allowed");
    compile_expression(code, data, a.dims.front());
    if (rt == RT_REAL)
      {
      convert_real_to_integer(code, data.stack_index);
      rt = RT_INTEGER;
      }
    code.add(VM::vmcode::SHL, FIRST_FREE_INTEGER_REG, VM::vmcode::NUMBER, 3);
    ++data.stack_index;
    update_data(data);
    compile_expression(code, data, a.expr);
    auto it = data.vars.find(a.name);
    if (it == data.vars.end())
      throw_compile_error(a.line_nr, "variable " + a.name + " is not declared.");
    if (it->second.st != constant)
      throw_compile_error(a.line_nr, "variable " + a.name + " cannot be assigned.");
    int64_t var_id = it->second.address;
    if (it->second.vt != single_array && it->second.vt != integer_array)
      throw_compile_error(a.line_nr, "I expect " + a.name + " to be array.");
    if (it->second.at != memory_address)
      throw_compile_error(a.line_nr, "I expect " + a.name + " to be an array on the stack.");
    if (it->second.vt == single_array && rt == RT_INTEGER)
      {
      convert_integer_to_real(code, data.stack_index);
      rt = RT_REAL;
      }
    else if (it->second.vt == integer_array && rt == RT_REAL)
      {
      convert_real_to_integer(code, data.stack_index);
      rt = RT_INTEGER;
      }
    --data.stack_index;
    //RAX contains the index
    //SECOND_FREE_REAL_REG or SECOND_FREE_INTEGER_REG contains the expression value
    switch (a.op.front())
      {
      case '=':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::SUB, VM::vmcode::RSP, FIRST_FREE_INTEGER_REG);
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, SECOND_FREE_REAL_REG);
        code.add(VM::vmcode::ADD, VM::vmcode::RSP, FIRST_FREE_INTEGER_REG);
        }
      else
        {
        code.add(VM::vmcode::SUB, VM::vmcode::RSP, FIRST_FREE_INTEGER_REG);
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, SECOND_FREE_INTEGER_REG);
        code.add(VM::vmcode::ADD, VM::vmcode::RSP, FIRST_FREE_INTEGER_REG);
        }
      break;
      }
      case '+':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::SUB, VM::vmcode::RSP, FIRST_FREE_INTEGER_REG);
        code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, -var_id);
        code.add(VM::vmcode::ADDSD, RESERVED_REAL_REG, SECOND_FREE_REAL_REG);
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, RESERVED_REAL_REG);
        code.add(VM::vmcode::ADD, VM::vmcode::RSP, FIRST_FREE_INTEGER_REG);
        }
      else
        {
        code.add(VM::vmcode::SUB, VM::vmcode::RSP, FIRST_FREE_INTEGER_REG);
        code.add(VM::vmcode::ADD, VM::vmcode::MEM_RSP, -var_id, SECOND_FREE_INTEGER_REG);
        code.add(VM::vmcode::ADD, VM::vmcode::RSP, FIRST_FREE_INTEGER_REG);
        }
      break;
      }
      case '-':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::SUB, VM::vmcode::RSP, FIRST_FREE_INTEGER_REG);
        code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, -var_id);
        code.add(VM::vmcode::SUBSD, RESERVED_REAL_REG, SECOND_FREE_REAL_REG);
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, RESERVED_REAL_REG);
        code.add(VM::vmcode::ADD, VM::vmcode::RSP, FIRST_FREE_INTEGER_REG);
        }
      else
        {
        code.add(VM::vmcode::SUB, VM::vmcode::RSP, FIRST_FREE_INTEGER_REG);
        code.add(VM::vmcode::SUB, VM::vmcode::MEM_RSP, -var_id, SECOND_FREE_INTEGER_REG);
        code.add(VM::vmcode::ADD, VM::vmcode::RSP, FIRST_FREE_INTEGER_REG);
        }
      break;
      }
      case '*':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::SUB, VM::vmcode::RSP, FIRST_FREE_INTEGER_REG);
        code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, -var_id);
        code.add(VM::vmcode::MULSD, RESERVED_REAL_REG, SECOND_FREE_REAL_REG);
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, RESERVED_REAL_REG);
        code.add(VM::vmcode::ADD, VM::vmcode::RSP, FIRST_FREE_INTEGER_REG);
        }
      else
        {
        code.add(VM::vmcode::SUB, VM::vmcode::RSP, FIRST_FREE_INTEGER_REG);
        code.add(VM::vmcode::IMUL, SECOND_FREE_INTEGER_REG, VM::vmcode::MEM_RSP, -var_id);
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, SECOND_FREE_INTEGER_REG);
        code.add(VM::vmcode::ADD, VM::vmcode::RSP, FIRST_FREE_INTEGER_REG);
        }
      break;
      }
      case '/':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::SUB, VM::vmcode::RSP, FIRST_FREE_INTEGER_REG);
        code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, -var_id);
        code.add(VM::vmcode::DIVSD, RESERVED_REAL_REG, SECOND_FREE_REAL_REG);
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, RESERVED_REAL_REG);
        code.add(VM::vmcode::ADD, VM::vmcode::RSP, FIRST_FREE_INTEGER_REG);
        }
      else
        {
        code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::RSP);
        code.add(VM::vmcode::SUB, STACK_BACKUP_REGISTER, FIRST_FREE_INTEGER_REG);
        //code.add(VM::vmcode::MOV, VM::vmcode::RAX, STACK_MEM_BACKUP_REGISTER, -var_id);
        code.add(VM::vmcode::IDIV2, STACK_MEM_BACKUP_REGISTER, -var_id, SECOND_FREE_INTEGER_REG);
        //code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, -var_id, VM::vmcode::RAX);
        }
      break;
      }
      default: throw std::runtime_error("not implemented");
      };
    }

  void compile_assignment_dereference(VM::vmcode& code, compile_data& data, const Assignment& a)
    {
    if (!a.dims.empty())
      throw_compile_error(a.line_nr, "dereference without dimensions expected");
    if (data.stack_index != 0)
      throw_compile_error(a.line_nr, "assignment only as single statement allowed");
    ++data.stack_index;
    update_data(data);
    compile_expression(code, data, a.expr);
    auto it = data.vars.find(a.name);
    if (it == data.vars.end())
      throw_compile_error(a.line_nr, "variable " + a.name + " is not declared.");
    int64_t var_id = it->second.address;
    if (it->second.vt != pointer_to_single && it->second.vt != pointer_to_integer)
      throw_compile_error(a.line_nr, "I expect " + a.name + " to be a pointer.");
    if (it->second.vt == pointer_to_single && rt == RT_INTEGER)
      {
      convert_integer_to_real(code, data.stack_index);
      rt = RT_REAL;
      }
    else if (it->second.vt == pointer_to_integer && rt == RT_REAL)
      {
      convert_real_to_integer(code, data.stack_index);
      rt = RT_INTEGER;
      }
    --data.stack_index;

    if (it->second.at == memory_address)
      code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::MEM_RSP, -var_id);
    else
      code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, (VM::vmcode::operand)var_id);
    //r10 contains the pointer address
    //SECOND_FREE_REAL_REG or SECOND_FREE_INTEGER_REG contains the expression value
    switch (a.op.front())
      {
      case '=':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, SECOND_FREE_REAL_REG);
        }
      else
        {
        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, SECOND_FREE_INTEGER_REG);
        }
      break;
      }
      case '+':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::MOV, RESERVED_REAL_REG, STACK_MEM_BACKUP_REGISTER);
        code.add(VM::vmcode::ADDSD, RESERVED_REAL_REG, SECOND_FREE_REAL_REG);
        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, RESERVED_REAL_REG);
        }
      else
        {
        code.add(VM::vmcode::ADD, STACK_MEM_BACKUP_REGISTER, SECOND_FREE_INTEGER_REG);
        }
      break;
      }
      case '-':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::MOV, RESERVED_REAL_REG, STACK_MEM_BACKUP_REGISTER);
        code.add(VM::vmcode::SUBSD, RESERVED_REAL_REG, SECOND_FREE_REAL_REG);
        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, RESERVED_REAL_REG);
        }
      else
        {
        code.add(VM::vmcode::SUB, STACK_MEM_BACKUP_REGISTER, SECOND_FREE_INTEGER_REG);
        }
      break;
      }
      case '*':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::MOV, RESERVED_REAL_REG, STACK_MEM_BACKUP_REGISTER);
        code.add(VM::vmcode::MULSD, RESERVED_REAL_REG, SECOND_FREE_REAL_REG);
        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, RESERVED_REAL_REG);
        }
      else
        {
        code.add(VM::vmcode::IMUL, SECOND_FREE_INTEGER_REG, STACK_MEM_BACKUP_REGISTER);
        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, SECOND_FREE_INTEGER_REG);
        }
      break;
      }
      case '/':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::MOV, RESERVED_REAL_REG, STACK_MEM_BACKUP_REGISTER);
        code.add(VM::vmcode::DIVSD, RESERVED_REAL_REG, SECOND_FREE_REAL_REG);
        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, RESERVED_REAL_REG);
        }
      else
        {
        //code.add(VM::vmcode::MOV, VM::vmcode::RAX, STACK_MEM_BACKUP_REGISTER);
        code.add(VM::vmcode::IDIV2, STACK_MEM_BACKUP_REGISTER, SECOND_FREE_INTEGER_REG);
        //code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, VM::vmcode::RAX);
        }
      break;
      }
      default: throw std::runtime_error("not implemented");
      };
    }

  void compile_assignment_single(VM::vmcode& code, compile_data& data, const Assignment& a)
    {
    compile_expression(code, data, a.expr);
    auto it = data.vars.find(a.name);
    if (it == data.vars.end())
      throw_compile_error(a.line_nr, "variable " + a.name + " is not declared.");
    if (it->second.st != constant)
      throw_compile_error(a.line_nr, "variable " + a.name + " cannot be assigned.");
    int64_t var_id = it->second.address;
    if (it->second.vt != single && it->second.vt != integer)
      throw_compile_error(a.line_nr, "I expect " + a.name + " to be an integer or a float.");
    if (it->second.vt == single && rt == RT_INTEGER)
      {
      convert_integer_to_real(code, data.stack_index);
      rt = RT_REAL;
      }
    else if (it->second.vt == integer && rt == RT_REAL)
      {
      convert_real_to_integer(code, data.stack_index);
      rt = RT_INTEGER;
      }
    VM::vmcode::operand int_op, real_op;
    int64_t int_offset, real_offset;
    index_to_integer_operand(int_op, int_offset, data.stack_index);
    index_to_real_operand(real_op, real_offset, data.stack_index);
    switch (a.op.front())
      {
      case '=':
      {
      if (rt == RT_REAL)
        {
        if (real_offset)
          {
          if (it->second.at == memory_address)
            {
            code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, real_offset);
            code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, RESERVED_INTEGER_REG);
            }
          else
            {
            code.add(VM::vmcode::MOV, (VM::vmcode::operand)var_id, VM::vmcode::MEM_RSP, real_offset);
            }
          }
        else
          {
          if (it->second.at == memory_address)
            code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, real_op);
          else
            code.add(VM::vmcode::MOV, (VM::vmcode::operand)var_id, real_op);
          }
        }
      else
        {
        if (it->second.at == memory_address)
          {
          if (int_offset)
            {
            code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, int_offset);
            int_op = RESERVED_INTEGER_REG;
            }
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, int_op);
          }
        else
          {
          if (int_offset)
            code.add(VM::vmcode::MOV, (VM::vmcode::operand)var_id, VM::vmcode::MEM_RSP, int_offset);
          else
            code.add(VM::vmcode::MOV, (VM::vmcode::operand)var_id, int_op);
          }
        }
      break;
      }
      case '+':
      {
      if (it->second.at == memory_address)
        {
        if (rt == RT_REAL)
          {
          if (real_offset)
            {
            code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, VM::vmcode::MEM_RSP, real_offset);
            real_op = RESERVED_REAL_REG_2;
            }
          code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, -var_id);
          code.add(VM::vmcode::ADDSD, RESERVED_REAL_REG, real_op);
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, RESERVED_REAL_REG);
          }
        else
          {
          if (int_offset)
            {
            code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, int_offset);
            int_op = RESERVED_INTEGER_REG;
            }
          code.add(VM::vmcode::ADD, VM::vmcode::MEM_RSP, -var_id, int_op);
          }
        }
      else
        {
        if (rt == RT_REAL)
          {
          if (real_offset)
            {
            code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, VM::vmcode::MEM_RSP, real_offset);
            real_op = RESERVED_REAL_REG_2;
            }
          code.add(VM::vmcode::ADDSD, (VM::vmcode::operand)var_id, real_op);
          }
        else
          {
          if (int_offset)
            {
            //code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, int_offset);
            //int_op = RESERVED_INTEGER_REG;
            code.add(VM::vmcode::ADD, (VM::vmcode::operand)var_id, VM::vmcode::MEM_RSP, int_offset);
            }
          else
            {
            code.add(VM::vmcode::ADD, (VM::vmcode::operand)var_id, int_op);
            }
          }
        }
      break;
      }
      case '-':
      {
      if (it->second.at == memory_address)
        {
        if (rt == RT_REAL)
          {
          if (real_offset)
            {
            code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, VM::vmcode::MEM_RSP, real_offset);
            real_op = RESERVED_REAL_REG_2;
            }
          code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, -var_id);
          code.add(VM::vmcode::SUBSD, RESERVED_REAL_REG, real_op);
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, RESERVED_REAL_REG);
          }
        else
          {
          if (int_offset)
            {
            code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, int_offset);
            int_op = RESERVED_INTEGER_REG;
            }
          code.add(VM::vmcode::SUB, VM::vmcode::MEM_RSP, -var_id, int_op);
          }
        }
      else
        {
        if (rt == RT_REAL)
          {
          if (real_offset)
            {
            code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, VM::vmcode::MEM_RSP, real_offset);
            real_op = RESERVED_REAL_REG_2;
            }
          code.add(VM::vmcode::SUBSD, (VM::vmcode::operand)var_id, real_op);
          }
        else
          {
          if (int_offset)
            {
            //code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, int_offset);
            //int_op = RESERVED_INTEGER_REG;
            code.add(VM::vmcode::SUB, (VM::vmcode::operand)var_id, VM::vmcode::MEM_RSP, int_offset);
            }
          else
            code.add(VM::vmcode::SUB, (VM::vmcode::operand)var_id, int_op);
          }
        }
      break;
      }
      case '*':
      {
      if (it->second.at == memory_address)
        {
        if (rt == RT_REAL)
          {
          if (real_offset)
            {
            code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, VM::vmcode::MEM_RSP, real_offset);
            real_op = RESERVED_REAL_REG_2;
            }
          code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, -var_id);
          code.add(VM::vmcode::MULSD, RESERVED_REAL_REG, real_op);
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, RESERVED_REAL_REG);
          }
        else
          {
          if (int_offset)
            {
            code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, int_offset);
            int_op = RESERVED_INTEGER_REG;
            }
          code.add(VM::vmcode::IMUL, int_op, VM::vmcode::MEM_RSP, -var_id);
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, int_op);
          }
        }
      else
        {
        if (rt == RT_REAL)
          {
          if (real_offset)
            {
            code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, VM::vmcode::MEM_RSP, real_offset);
            real_op = RESERVED_REAL_REG_2;
            }
          code.add(VM::vmcode::MULSD, (VM::vmcode::operand)var_id, real_op);
          }
        else
          {
          if (int_offset)
            {
            code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, int_offset);
            int_op = RESERVED_INTEGER_REG;
            }
          code.add(VM::vmcode::IMUL, (VM::vmcode::operand)var_id, int_op);
          }
        }
      break;
      }
      case '/':
      {
      if (it->second.at == memory_address)
        {
        if (rt == RT_REAL)
          {
          if (real_offset)
            {
            code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, VM::vmcode::MEM_RSP, real_offset);
            real_op = RESERVED_REAL_REG_2;
            }
          code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, -var_id);
          code.add(VM::vmcode::DIVSD, RESERVED_REAL_REG, real_op);
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, RESERVED_REAL_REG);
          }
        else
          {
          if (int_op == VM::vmcode::RAX)
            {
            code.add(VM::vmcode::MOV, SECOND_FREE_INTEGER_REG, VM::vmcode::RAX);
            //code.add(VM::vmcode::MOV, VM::vmcode::RAX, VM::vmcode::MEM_RSP, -var_id);
            code.add(VM::vmcode::IDIV2, VM::vmcode::MEM_RSP, -var_id, SECOND_FREE_INTEGER_REG);
            //code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::RAX);
            }
          else
            {
            throw std::runtime_error("not implemented");
            }
          }
        }
      else
        {
        if (rt == RT_REAL)
          {
          if (real_offset)
            {
            code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, VM::vmcode::MEM_RSP, real_offset);
            real_op = RESERVED_REAL_REG_2;
            }
          code.add(VM::vmcode::DIVSD, (VM::vmcode::operand)var_id, real_op);
          }
        else
          {
          if (int_op == VM::vmcode::RAX)
            {
            code.add(VM::vmcode::MOV, SECOND_FREE_INTEGER_REG, VM::vmcode::RAX);
            //code.add(VM::vmcode::MOV, VM::vmcode::RAX, (VM::vmcode::operand)var_id);
            code.add(VM::vmcode::IDIV2, (VM::vmcode::operand)var_id, SECOND_FREE_INTEGER_REG);
            //code.add(VM::vmcode::MOV, (VM::vmcode::operand)var_id, VM::vmcode::RAX);
            }
          else
            {
            throw std::runtime_error("not implemented");
            }
          }
        }
      break;
      }
      default: throw std::runtime_error("not implemented");
      }
    }

  void compile_assignment(VM::vmcode& code, compile_data& data, const Assignment& a)
    {
    if (data.stack_index != 0)
      throw_compile_error(a.line_nr, "assignment only as single statement allowed");

    auto it = data.vars.find(a.name);
    if (it == data.vars.end())
      throw_compile_error(a.line_nr, "variable " + a.name + " is not declared.");

    if (!a.dims.empty())
      {
      if (it->second.vt == pointer_to_single || it->second.vt == pointer_to_integer)
        compile_assignment_pointer(code, data, a);
      else
        compile_assignment_array(code, data, a);
      }
    else if (a.dereference)
      compile_assignment_dereference(code, data, a);
    else
      compile_assignment_single(code, data, a);
    }


  void compile_array_call(VM::vmcode& code, compile_data& data, const ArrayCall& a)
    {
    auto it = data.vars.find(a.name);
    if (it == data.vars.end())
      throw_compile_error(a.line_nr, "variable " + a.name + " is unknown");
    if (a.exprs.size() != 1)
      throw_compile_error(a.line_nr, "only single dimension arrays are allowed.");
    if (it->second.vt == single || it->second.vt == integer)
      throw_compile_error(a.line_nr, "I expect an array or a pointer.");
    compile_expression(code, data, a.exprs.front());
    if (rt == RT_REAL)
      {
      convert_real_to_integer(code, data.stack_index);
      rt = RT_INTEGER;
      }
    int64_t var_id = it->second.address;

    if (it->second.vt == single_array)
      {
      if (it->second.at != memory_address)
        throw_compile_error(a.line_nr, "I expect " + a.name + " to be an array on the stack.");
      VM::vmcode::operand op;
      int64_t offset;
      index_to_integer_operand(op, offset, data.stack_index);
      if (offset)
        {
        code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset);
        op = RESERVED_INTEGER_REG;
        }
      code.add(VM::vmcode::SHL, op, VM::vmcode::NUMBER, 3);
      code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::RSP);
      code.add(VM::vmcode::SUB, STACK_BACKUP_REGISTER, op);
      index_to_real_operand(op, offset, data.stack_index);
      code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, STACK_MEM_BACKUP_REGISTER, -var_id);
      if (offset)
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, RESERVED_INTEGER_REG);
      else
        code.add(VM::vmcode::MOV, op, RESERVED_INTEGER_REG);
      rt = RT_REAL;
      }
    else if (it->second.vt == integer_array)
      {
      if (it->second.at != memory_address)
        throw_compile_error(a.line_nr, "I expect " + a.name + " to be an array on the stack.");
      VM::vmcode::operand op;
      int64_t offset;
      index_to_integer_operand(op, offset, data.stack_index);
      if (offset)
        {
        code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset);
        op = RESERVED_INTEGER_REG;
        }
      code.add(VM::vmcode::SHL, op, VM::vmcode::NUMBER, 3);
      code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::RSP);
      code.add(VM::vmcode::SUB, STACK_BACKUP_REGISTER, op);
      if (offset)
        {
        code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, STACK_MEM_BACKUP_REGISTER, -var_id);
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, RESERVED_INTEGER_REG);
        }
      else
        code.add(VM::vmcode::MOV, op, STACK_MEM_BACKUP_REGISTER, -var_id);
      rt = RT_INTEGER;
      }
    else if (it->second.vt == pointer_to_single)
      {
      if (it->second.at == memory_address)
        code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::MEM_RSP, -var_id); // address to pointer
      else
        code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, (VM::vmcode::operand)var_id); // address to pointer
      VM::vmcode::operand op;
      int64_t offset;
      index_to_integer_operand(op, offset, data.stack_index);
      if (offset)
        {
        code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset);
        code.add(VM::vmcode::SHL, RESERVED_INTEGER_REG, VM::vmcode::NUMBER, 3);
        code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, RESERVED_INTEGER_REG);
        }
      else
        {
        code.add(VM::vmcode::SHL, op, VM::vmcode::NUMBER, 3);
        code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, op);
        }
      index_to_real_operand(op, offset, data.stack_index);
      if (offset)
        {
        code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, STACK_MEM_BACKUP_REGISTER);
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, RESERVED_INTEGER_REG);
        }
      else
        {
        code.add(VM::vmcode::MOV, op, STACK_MEM_BACKUP_REGISTER);
        }
      rt = RT_REAL;
      }
    else if (it->second.vt == pointer_to_integer)
      {
      if (it->second.at == memory_address)
        code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::MEM_RSP, -var_id); // address to pointer
      else
        code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, (VM::vmcode::operand)var_id); // address to pointer
      VM::vmcode::operand op;
      int64_t offset;
      index_to_integer_operand(op, offset, data.stack_index);
      if (offset)
        {
        code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset);
        code.add(VM::vmcode::SHL, RESERVED_INTEGER_REG, VM::vmcode::NUMBER, 3);
        code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, RESERVED_INTEGER_REG);
        code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, STACK_MEM_BACKUP_REGISTER);
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, RESERVED_INTEGER_REG);
        }
      else
        {
        code.add(VM::vmcode::SHL, op, VM::vmcode::NUMBER, 3);
        code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, op);
        code.add(VM::vmcode::MOV, op, STACK_MEM_BACKUP_REGISTER);
        }
      rt = RT_INTEGER;
      }
    else
      throw std::runtime_error("not implemented");
    }

  void compile_dereference(VM::vmcode& code, compile_data& data, const Dereference& d)
    {
    auto it = data.vars.find(d.name);
    if (it == data.vars.end())
      throw_compile_error(d.line_nr, "variable " + d.name + " is unknown");
    if (it->second.vt != pointer_to_single && it->second.vt != pointer_to_integer)
      throw_compile_error(d.line_nr, "I expect a pointer to dereference.");
    int64_t var_id = it->second.address;
    if (it->second.at == memory_address)
      code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::MEM_RSP, -var_id);
    else
      code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, (VM::vmcode::operand)var_id);
    if (it->second.vt == pointer_to_single)
      {
      VM::vmcode::operand op;
      int64_t offset;
      index_to_real_operand(op, offset, data.stack_index);
      if (offset)
        {
        code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, STACK_MEM_BACKUP_REGISTER);
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, RESERVED_INTEGER_REG);
        }
      else
        {
        code.add(VM::vmcode::MOV, op, STACK_MEM_BACKUP_REGISTER);
        }
      rt = RT_REAL;
      }
    else
      {
      VM::vmcode::operand op;
      int64_t offset;
      index_to_integer_operand(op, offset, data.stack_index);
      if (offset)
        {
        code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, STACK_MEM_BACKUP_REGISTER);
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, RESERVED_INTEGER_REG);
        }
      else
        code.add(VM::vmcode::MOV, op, STACK_MEM_BACKUP_REGISTER);
      rt = RT_INTEGER;
      }
    }

  void compile_real(VM::vmcode& code, compile_data& data, const value_t& v)
    {
    double f = (double)std::get<double>(v);
    VM::vmcode::operand op;
    int64_t offset;
    index_to_real_operand(op, offset, data.stack_index);
    if (offset)
      {
      code.add(VM::vmcode::MOV, op, offset, VM::vmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&f)));
      }
    else
      {
      if (f)
        {
        code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&f)));
        code.add(VM::vmcode::MOV, op, RESERVED_INTEGER_REG);
        }
      else
        {
        code.add(VM::vmcode::XORPD, op, op);
        }
      }
    rt = RT_REAL;
    }

  void compile_integer(VM::vmcode& code, compile_data& data, const value_t& v)
    {
    int64_t i = (int64_t)std::get<int64_t>(v);
    VM::vmcode::operand op;
    int64_t offset;
    index_to_integer_operand(op, offset, data.stack_index);
    if (offset)
      {
      code.add(VM::vmcode::MOV, op, offset, VM::vmcode::NUMBER, i);
      }
    else
      {
      if (i)
        code.add(VM::vmcode::MOV, op, VM::vmcode::NUMBER, i);
      else
        code.add(VM::vmcode::XOR, op, op);
      }
    rt = RT_INTEGER;
    }

  void compile_value(VM::vmcode& code, compile_data& data, const value_t& v)
    {
    if (std::holds_alternative<double>(v))
      compile_real(code, data, v);
    else
      compile_integer(code, data, v);
    }


  void compile_lvalue_operator(VM::vmcode& code, compile_data& data, const LValueOperator& lvo)
    {
    if (std::holds_alternative<Variable>(lvo.lvalue->lvalue))
      {
      // TODO VARIABLE REG STUFF
      Variable v = std::get<Variable>(lvo.lvalue->lvalue);
      auto it = data.vars.find(v.name);
      if (it == data.vars.end())
        throw_compile_error(v.line_nr, "Cannot find variable " + v.name);
      uint64_t var_id = it->second.address;
      if (it->second.st != constant)
        throw_compile_error(v.line_nr, "Can only change constant space variables");
      rt = (it->second.vt == single) ? RT_REAL : RT_INTEGER;
      if (rt == RT_INTEGER)
        {
        if (lvo.name == "++")
          {
          if (it->second.at == memory_address)
            code.add(VM::vmcode::ADD, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::NUMBER, 1);
          else
            code.add(VM::vmcode::INC, (VM::vmcode::operand)var_id);
          }
        else if (lvo.name == "--")
          {
          if (it->second.at == memory_address)
            code.add(VM::vmcode::SUB, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::NUMBER, 1);
          else
            code.add(VM::vmcode::DEC, (VM::vmcode::operand)var_id);
          }
        else
          throw_compile_error(lvo.line_nr, "operator " + lvo.name + " is unknown");
        VM::vmcode::operand op;
        int64_t offset;
        index_to_integer_operand(op, offset, data.stack_index);
        if (it->second.at == memory_address)
          {
          if (offset)
            {
            code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, -var_id);
            code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, RESERVED_INTEGER_REG);
            }
          else
            code.add(VM::vmcode::MOV, op, VM::vmcode::MEM_RSP, -var_id);
          }
        else
          {
          if (offset)
            code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, (VM::vmcode::operand)var_id);
          else
            code.add(VM::vmcode::MOV, op, (VM::vmcode::operand)var_id);
          }
        }
      else
        {
        if (it->second.at == memory_address)
          {
          VM::vmcode::operand op;
          int64_t offset;
          index_to_real_operand(op, offset, data.stack_index);
          if (offset)
            {
            op = RESERVED_REAL_REG;
            code.add(VM::vmcode::MOV, RESERVED_REAL_REG, VM::vmcode::MEM_RSP, -var_id);
            }
          else
            code.add(VM::vmcode::MOV, op, VM::vmcode::MEM_RSP, -var_id);
          double f = 1.0;
          code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&f)));
          code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, RESERVED_INTEGER_REG);
          if (lvo.name == "++")
            code.add(VM::vmcode::ADDSD, op, RESERVED_REAL_REG_2);
          else if (lvo.name == "--")
            code.add(VM::vmcode::SUBSD, op, RESERVED_REAL_REG_2);
          else
            throw_compile_error(lvo.line_nr, "operator " + lvo.name + " is unknown");
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, op);
          }
        else
          {
          double f = 1.0;
          code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&f)));
          code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, RESERVED_INTEGER_REG);
          if (lvo.name == "++")
            code.add(VM::vmcode::ADDSD, (VM::vmcode::operand)var_id, RESERVED_REAL_REG_2);
          else if (lvo.name == "--")
            code.add(VM::vmcode::SUBSD, (VM::vmcode::operand)var_id, RESERVED_REAL_REG_2);
          else
            throw_compile_error(lvo.line_nr, "operator " + lvo.name + " is unknown");
          VM::vmcode::operand op;
          int64_t offset;
          index_to_real_operand(op, offset, data.stack_index);
          if (offset == 0 && op == FIRST_FREE_REAL_REG)
            code.add(VM::vmcode::MOV, FIRST_FREE_REAL_REG, (VM::vmcode::operand)var_id); // send through the output to the output register
          }
        }
      }
    else if (std::holds_alternative<ArrayCall>(lvo.lvalue->lvalue))
      {
      ArrayCall a = std::get<ArrayCall>(lvo.lvalue->lvalue);
      auto it = data.vars.find(a.name);
      if (it == data.vars.end())
        throw_compile_error(a.line_nr, "Cannot find variable " + a.name);
      compile_expression(code, data, a.exprs.front());
      if (rt == RT_REAL)
        {
        convert_real_to_integer(code, data.stack_index);
        rt = RT_INTEGER;
        }
      int64_t var_id = it->second.address;

      if (it->second.vt == single_array)
        {
        if (it->second.at != memory_address)
          throw_compile_error(a.line_nr, "I expect " + a.name + " to be an array on the stack.");
        VM::vmcode::operand op;
        int64_t offset;
        index_to_integer_operand(op, offset, data.stack_index);
        if (offset)
          {
          code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset);
          op = RESERVED_INTEGER_REG;
          }
        code.add(VM::vmcode::SHL, op, VM::vmcode::NUMBER, 3);
        code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::RSP);
        code.add(VM::vmcode::SUB, STACK_BACKUP_REGISTER, op);
        index_to_real_operand(op, offset, data.stack_index);

        if (offset)
          {
          op = RESERVED_REAL_REG;
          code.add(VM::vmcode::MOV, RESERVED_REAL_REG, STACK_MEM_BACKUP_REGISTER, -var_id);
          }
        else
          code.add(VM::vmcode::MOV, op, STACK_MEM_BACKUP_REGISTER, -var_id);
        double f = 1.0;
        code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&f)));
        code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, RESERVED_INTEGER_REG);
        if (lvo.name == "++")
          code.add(VM::vmcode::ADDSD, op, RESERVED_REAL_REG_2);
        else if (lvo.name == "--")
          code.add(VM::vmcode::SUBSD, op, RESERVED_REAL_REG_2);
        else
          throw_compile_error(lvo.line_nr, "operator " + lvo.name + " is unknown");

        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, -var_id, op);
        rt = RT_REAL;
        }
      else if (it->second.vt == integer_array)
        {
        if (it->second.at != memory_address)
          throw_compile_error(a.line_nr, "I expect " + a.name + " to be an array on the stack.");
        VM::vmcode::operand op;
        int64_t offset;
        index_to_integer_operand(op, offset, data.stack_index);
        if (offset)
          {
          code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset);
          op = RESERVED_INTEGER_REG;
          }
        code.add(VM::vmcode::SHL, op, VM::vmcode::NUMBER, 3);
        code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::RSP);
        code.add(VM::vmcode::SUB, STACK_BACKUP_REGISTER, op);
        if (lvo.name == "++")
          code.add(VM::vmcode::ADD, STACK_MEM_BACKUP_REGISTER, -var_id, VM::vmcode::NUMBER, 1);
        else if (lvo.name == "--")
          code.add(VM::vmcode::SUB, STACK_MEM_BACKUP_REGISTER, -var_id, VM::vmcode::NUMBER, 1);
        else
          throw_compile_error(lvo.line_nr, "operator " + lvo.name + " is unknown");
        if (offset)
          {
          code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, STACK_MEM_BACKUP_REGISTER, -var_id);
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, RESERVED_INTEGER_REG);
          }
        else
          code.add(VM::vmcode::MOV, op, STACK_MEM_BACKUP_REGISTER, -var_id);
        rt = RT_INTEGER;
        }
      else if (it->second.vt == pointer_to_single)
        {
        if (it->second.at == memory_address)
          code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::MEM_RSP, -var_id); // address to pointer
        else
          code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, (VM::vmcode::operand)var_id); // address to pointer
        VM::vmcode::operand op;
        int64_t offset;
        index_to_integer_operand(op, offset, data.stack_index);
        if (offset)
          {
          code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset);
          code.add(VM::vmcode::SHL, RESERVED_INTEGER_REG, VM::vmcode::NUMBER, 3);
          code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, RESERVED_INTEGER_REG);
          }
        else
          {
          code.add(VM::vmcode::SHL, op, VM::vmcode::NUMBER, 3);
          code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, op);
          }
        index_to_real_operand(op, offset, data.stack_index);
        if (offset)
          {
          op = RESERVED_REAL_REG;
          code.add(VM::vmcode::MOV, RESERVED_REAL_REG, STACK_MEM_BACKUP_REGISTER);
          }
        else
          code.add(VM::vmcode::MOV, op, STACK_MEM_BACKUP_REGISTER);
        double f = 1.0;
        code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&f)));
        code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, RESERVED_INTEGER_REG);
        if (lvo.name == "++")
          code.add(VM::vmcode::ADDSD, op, RESERVED_REAL_REG_2);
        else if (lvo.name == "--")
          code.add(VM::vmcode::SUBSD, op, RESERVED_REAL_REG_2);
        else
          throw_compile_error(lvo.line_nr, "operator " + lvo.name + " is unknown");

        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, op);
        rt = RT_REAL;
        }
      else if (it->second.vt == pointer_to_integer)
        {
        if (it->second.at == memory_address)
          code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::MEM_RSP, -var_id); // address to pointer
        else
          code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, (VM::vmcode::operand)var_id); // address to pointer
        VM::vmcode::operand op;
        int64_t offset;
        index_to_integer_operand(op, offset, data.stack_index);
        if (offset)
          {
          code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::MEM_RSP, offset);
          code.add(VM::vmcode::SHL, RESERVED_INTEGER_REG, VM::vmcode::NUMBER, 3);
          code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, RESERVED_INTEGER_REG);
          }
        else
          {
          code.add(VM::vmcode::SHL, op, VM::vmcode::NUMBER, 3);
          code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, op);
          }
        if (lvo.name == "++")
          code.add(VM::vmcode::ADD, STACK_MEM_BACKUP_REGISTER, VM::vmcode::NUMBER, 1);
        else if (lvo.name == "--")
          code.add(VM::vmcode::SUB, STACK_MEM_BACKUP_REGISTER, VM::vmcode::NUMBER, 1);
        else
          throw_compile_error(lvo.line_nr, "operator " + lvo.name + " is unknown");
        if (offset)
          {
          code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, STACK_MEM_BACKUP_REGISTER);
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, RESERVED_INTEGER_REG);
          }
        else
          code.add(VM::vmcode::MOV, op, STACK_MEM_BACKUP_REGISTER);
        rt = RT_INTEGER;
        }
      else
        throw std::runtime_error("not implemented");
      }
    else if (std::holds_alternative<Dereference>(lvo.lvalue->lvalue))
      {
      Dereference d = std::get<Dereference>(lvo.lvalue->lvalue);
      auto it = data.vars.find(d.name);
      if (it == data.vars.end())
        throw_compile_error(d.line_nr, "Cannot find variable " + d.name);
      if (it->second.vt != pointer_to_single && it->second.vt != pointer_to_integer)
        throw_compile_error(d.line_nr, "I expect a pointer to dereference.");
      int64_t var_id = it->second.address;
      if (it->second.vt == pointer_to_single)
        {
        if (it->second.at == memory_address)
          code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::MEM_RSP, -var_id); // address to pointer
        else
          code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, (VM::vmcode::operand)var_id); // address to pointer
        VM::vmcode::operand op;
        int64_t offset;
        index_to_real_operand(op, offset, data.stack_index);
        if (offset)
          {
          op = RESERVED_REAL_REG;
          code.add(VM::vmcode::MOV, RESERVED_REAL_REG, STACK_MEM_BACKUP_REGISTER);
          }
        else
          code.add(VM::vmcode::MOV, op, STACK_MEM_BACKUP_REGISTER);
        double f = 1.0;
        code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, VM::vmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&f)));
        code.add(VM::vmcode::MOV, RESERVED_REAL_REG_2, RESERVED_INTEGER_REG);
        if (lvo.name == "++")
          code.add(VM::vmcode::ADDSD, op, RESERVED_REAL_REG_2);
        else if (lvo.name == "--")
          code.add(VM::vmcode::SUBSD, op, RESERVED_REAL_REG_2);
        else
          throw_compile_error(lvo.line_nr, "operator " + lvo.name + " is unknown");

        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, op);
        rt = RT_REAL;
        }
      else if (it->second.vt == pointer_to_integer)
        {
        if (it->second.at == memory_address)
          code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::MEM_RSP, -var_id); // address to pointer
        else
          code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, (VM::vmcode::operand)var_id); // address to pointer
        VM::vmcode::operand op;
        int64_t offset;
        index_to_integer_operand(op, offset, data.stack_index);
        if (lvo.name == "++")
          code.add(VM::vmcode::ADD, STACK_MEM_BACKUP_REGISTER, VM::vmcode::NUMBER, 1);
        else if (lvo.name == "--")
          code.add(VM::vmcode::SUB, STACK_MEM_BACKUP_REGISTER, VM::vmcode::NUMBER, 1);
        else
          throw_compile_error(lvo.line_nr, "operator " + lvo.name + " is unknown");
        if (offset)
          {
          code.add(VM::vmcode::MOV, RESERVED_INTEGER_REG, STACK_MEM_BACKUP_REGISTER);
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, RESERVED_INTEGER_REG);
          }
        else
          code.add(VM::vmcode::MOV, op, STACK_MEM_BACKUP_REGISTER);
        rt = RT_INTEGER;
        }
      else
        throw std::runtime_error("not implemented");
      }
    else
      throw_compile_error("compile_lvalue_operator: not implemented");
    }

  void compile_funccall(VM::vmcode& code, compile_data& data, const FuncCall& f)
    {
    auto stack_save = data.stack_index;
    for (size_t i = 0; i < f.exprs.size(); ++i)
      {
      compile_expression(code, data, f.exprs[i]);
      if (rt == RT_INTEGER)
        {
        convert_integer_to_real(code, data.stack_index);
        rt = RT_REAL;
        }
      ++data.stack_index;
      }
    data.stack_index = stack_save;
    auto it = c_funcs.find(f.name);
    if (it == c_funcs.end())
      throw_compile_error(f.line_nr, "Invalid function");
    it->second(code, data);
    }

  void compile_factor(VM::vmcode& code, compile_data& data, const Factor& f)
    {
    if (std::holds_alternative<value_t>(f.factor))
      {
      compile_value(code, data, std::get<value_t>(f.factor));
      }
    else if (std::holds_alternative<Expression>(f.factor))
      {
      compile_expression(code, data, std::get<Expression>(f.factor));
      }
    else if (std::holds_alternative<Variable>(f.factor))
      {
      compile_variable(code, data, std::get<Variable>(f.factor));
      }
    else if (std::holds_alternative<ArrayCall>(f.factor))
      {
      compile_array_call(code, data, std::get<ArrayCall>(f.factor));
      }
    else if (std::holds_alternative<Dereference>(f.factor))
      {
      compile_dereference(code, data, std::get<Dereference>(f.factor));
      }
    else if (std::holds_alternative<LValueOperator>(f.factor))
      {
      compile_lvalue_operator(code, data, std::get<LValueOperator>(f.factor));
      }
    else if (std::holds_alternative<FuncCall>(f.factor))
      {
      compile_funccall(code, data, std::get<FuncCall>(f.factor));
      }
    else
      throw std::runtime_error("compile_factor: not implemented");
    if (f.sign == '-')
      {
      if (rt == RT_INTEGER)
        change_sign_integer(code, data);
      else
        change_sign_real(code, data);
      }
    }

  void compile_term(VM::vmcode& code, compile_data& data, const Term& t)
    {
    compile_factor(code, data, t.operands[0]);
    auto target_type = rt;
    for (size_t i = 0; i < t.fops.size(); ++i)
      {
      ++data.stack_index;
      update_data(data);
      compile_factor(code, data, t.operands[i + 1]);
      if (rt != target_type)
        {
        if (rt == RT_INTEGER)
          {
          convert_integer_to_real(code, data.stack_index);
          rt = RT_REAL;
          }
        else
          {
          convert_integer_to_real(code, data.stack_index - 1);
          target_type = RT_REAL;
          }
        }
      auto it = c_funcs.find(t.fops[i] + std::string(target_type == RT_INTEGER ? "i" : ""));
      if (it == c_funcs.end())
        throw_compile_error(t.line_nr, "Invalid operator");
      it->second(code, data);
      --data.stack_index;
      }
    }

  void compile_relop(VM::vmcode& code, compile_data& data, const Relop& r)
    {
    compile_term(code, data, r.operands[0]);
    auto target_type = rt;
    for (size_t i = 0; i < r.fops.size(); ++i)
      {
      ++data.stack_index;
      update_data(data);
      compile_term(code, data, r.operands[i + 1]);
      if (rt != target_type)
        {
        if (rt == RT_INTEGER)
          {
          convert_integer_to_real(code, data.stack_index);
          rt = RT_REAL;
          }
        else
          {
          convert_integer_to_real(code, data.stack_index - 1);
          target_type = RT_REAL;
          }
        }
      auto it = c_funcs.find(r.fops[i] + std::string(target_type == RT_INTEGER ? "i" : ""));
      if (it == c_funcs.end())
        throw_compile_error(r.line_nr, "Invalid operator");
      it->second(code, data);
      --data.stack_index;
      }
    }

  void compile_expression(VM::vmcode& code, compile_data& data, const Expression& expr)
    {
    compile_relop(code, data, expr.operands[0]);
    auto target_type = rt;
    for (size_t i = 0; i < expr.fops.size(); ++i)
      {
      ++data.stack_index;
      update_data(data);
      compile_relop(code, data, expr.operands[i + 1]);
      if (rt != target_type)
        {
        if (rt == RT_INTEGER)
          {
          convert_integer_to_real(code, data.stack_index);
          rt = RT_REAL;
          }
        else
          {
          convert_integer_to_real(code, data.stack_index - 1);
          target_type = RT_REAL;
          }
        }
      auto it = c_funcs.find(expr.fops[i] + std::string(target_type == RT_INTEGER ? "i" : ""));
      if (it == c_funcs.end())
        throw_compile_error(expr.line_nr, "Invalid operator");
      it->second(code, data);
      --data.stack_index;
      }
    }

  void compile_for_condition(VM::vmcode& code, compile_data& data, const Statement& stm, uint64_t end_label, int line_nr)
    {
    if (is_simple_relative_operation(stm))
      {
      if (data.stack_index != 0)
        throw_compile_error(line_nr, "for loop not expected as an expression");
      const Expression& expr = std::get<Expression>(stm);
      assert(expr.fops.size() == 1);
      assert(expr.operands.size() == 2);
      compile_relop(code, data, expr.operands[0]);
      auto target_type = rt;
      ++data.stack_index;
      update_data(data);
      compile_relop(code, data, expr.operands[1]);
      if (rt != target_type)
        {
        if (rt == RT_INTEGER)
          {
          convert_integer_to_real(code, data.stack_index);
          rt = RT_REAL;
          }
        else
          {
          convert_integer_to_real(code, data.stack_index - 1);
          target_type = RT_REAL;
          }
        }
      if (target_type == RT_REAL)
        {
        auto it = c_funcs.find(expr.fops[0]);
        if (it == c_funcs.end())
          throw_compile_error(expr.line_nr, "Invalid operator");
        it->second(code, data);
        --data.stack_index;
        code.add(VM::vmcode::CMP, FIRST_FREE_INTEGER_REG, VM::vmcode::NUMBER, 0);
        code.add(VM::vmcode::JE, label_to_string(end_label));
        }
      else
        {
        --data.stack_index;
        code.add(VM::vmcode::CMP, FIRST_FREE_INTEGER_REG, SECOND_FREE_INTEGER_REG);
        if (expr.fops[0] == "<")
          code.add(VM::vmcode::JGE, label_to_string(end_label));
        else if (expr.fops[0] == "<=")
          code.add(VM::vmcode::JG, label_to_string(end_label));
        else if (expr.fops[0] == ">")
          code.add(VM::vmcode::JLE, label_to_string(end_label));
        else if (expr.fops[0] == ">=")
          code.add(VM::vmcode::JL, label_to_string(end_label));
        else if (expr.fops[0] == "==")
          code.add(VM::vmcode::JNE, label_to_string(end_label));
        else if (expr.fops[0] == "!=")
          code.add(VM::vmcode::JE, label_to_string(end_label));
        else
          throw_compile_error(line_nr, "Invalid operator");
        }
      }
    else
      {
      compile_statement(code, data, stm);
      if (rt != RT_INTEGER)
        throw_compile_error(line_nr, "for loop condition is not a boolean expression");
      if (data.stack_index != 0)
        throw_compile_error(line_nr, "for loop not expected as an expression");
      code.add(VM::vmcode::CMP, FIRST_FREE_INTEGER_REG, VM::vmcode::NUMBER, 0);
      code.add(VM::vmcode::JE, label_to_string(end_label));
      }
    }

  void compile_for(VM::vmcode& code, compile_data& data, const For& f)
    {
    compile_statement(code, data, f.init_cond_inc[0]);
    auto start = data.label++;
    auto end = data.label++;
    code.add(VM::vmcode::LABEL, label_to_string(start));
    compile_for_condition(code, data, f.init_cond_inc[1], end, f.line_nr);
    for (const auto& stm : f.statements)
      compile_statement(code, data, stm);
    compile_statement(code, data, f.init_cond_inc[2]);
    code.add(VM::vmcode::JMP, label_to_string(start));
    code.add(VM::vmcode::LABEL, label_to_string(end));
    }

  void compile_statement(VM::vmcode& code, compile_data& data, const Statement& stm)
    {
    if (std::holds_alternative<Expression>(stm))
      {
      compile_expression(code, data, std::get<Expression>(stm));
      }
    else if (std::holds_alternative<Float>(stm))
      {
      compile_make_float(code, data, std::get<Float>(stm));
      }
    else if (std::holds_alternative<Int>(stm))
      {
      compile_make_int(code, data, std::get<Int>(stm));
      }
    else if (std::holds_alternative<Assignment>(stm))
      {
      compile_assignment(code, data, std::get<Assignment>(stm));
      }
    else if (std::holds_alternative<For>(stm))
      {
      compile_for(code, data, std::get<For>(stm));
      }
    else if (std::holds_alternative<Nop>(stm))
      {
      }
    else
      throw std::runtime_error("compile_statement: not implemented");
    }

  void adapt_offset_stack(VM::vmcode::operand& op, uint64_t& mem, compile_data& data)
    {
    if ((op == VM::vmcode::MEM_RSP || op == STACK_MEM_BACKUP_REGISTER) && ((mem & variable_tag) == variable_tag))
      {
      mem = mem & 0xFFFFFFFFFFFFFFF8;
      if (data.max_stack_index)
        mem -= (data.max_stack_index - 1) * 8;
      }
    }

  void offset_stack(VM::vmcode& code, compile_data& data)
    {
    auto& lst = code.get_instructions_list();
    for (auto& v : lst)
      {
      for (auto& instr : v)
        {
        adapt_offset_stack(instr.operand1, instr.operand1_mem, data);
        adapt_offset_stack(instr.operand2, instr.operand2_mem, data);
        }
      }
    }

  void compile_int_parameter_pointer(VM::vmcode& code, compile_data& data, const IntParameter& ip, int parameter_id)
    {
    auto it = data.vars.find(ip.name);
    if (it != data.vars.end())
      throw_compile_error(ip.line_nr, "Variable " + ip.name + " already exists");

    if (parameter_id < 4)
      {
      VM::vmcode::operand op;
      switch (parameter_id)
        {
        case 0: op = CALLING_CONVENTION_INT_PAR_1; break;
        case 1: op = CALLING_CONVENTION_INT_PAR_2; break;
        case 2: op = CALLING_CONVENTION_INT_PAR_3; break;
        case 3: op = CALLING_CONVENTION_INT_PAR_4; break;
        default:
          throw_compile_error("calling convention error");
        }
      assert(data.ra.is_free_integer_register(op));
      data.vars.insert(std::make_pair(ip.name, make_variable(op, external, pointer_to_integer, register_address)));
      data.ra.make_integer_register_unavailable(op);
      }
    else
      {
      int64_t var_id = data.var_offset | variable_tag;
      data.var_offset += 8;
      int addr = parameter_id - 4;
      code.add(VM::vmcode::MOV, VM::vmcode::RAX, VM::vmcode::MEM_RSP, rsp_offset + addr * 8 + virtual_machine_rsp_offset);
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::RAX);
      data.vars.insert(std::make_pair(ip.name, make_variable(var_id, external, pointer_to_integer, memory_address)));
      }
    }

  void compile_int_parameter_single(VM::vmcode& code, compile_data& data, const IntParameter& ip, int parameter_id)
    {
    auto it = data.vars.find(ip.name);
    if (it != data.vars.end())
      throw_compile_error(ip.line_nr, "Variable " + ip.name + " already exists");

    if (parameter_id < 4)
      {
      VM::vmcode::operand op;
      switch (parameter_id)
        {
        case 0: op = CALLING_CONVENTION_INT_PAR_1; break;
        case 1: op = CALLING_CONVENTION_INT_PAR_2; break;
        case 2: op = CALLING_CONVENTION_INT_PAR_3; break;
        case 3: op = CALLING_CONVENTION_INT_PAR_4; break;
        default:
          throw_compile_error("calling convention error");
        }
      assert(data.ra.is_free_integer_register(op));
      data.vars.insert(std::make_pair(ip.name, make_variable(op, constant, integer, register_address)));
      data.ra.make_integer_register_unavailable(op);
      }
    else
      {
      int64_t var_id = data.var_offset | variable_tag;
      data.var_offset += 8;
      int addr = parameter_id - 4;
      code.add(VM::vmcode::MOV, VM::vmcode::RAX, VM::vmcode::MEM_RSP, rsp_offset + addr * 8 + virtual_machine_rsp_offset);
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::RAX);
      data.vars.insert(std::make_pair(ip.name, make_variable(var_id, constant, integer, memory_address)));
      }
    }

  void compile_int_parameter(VM::vmcode& code, compile_data& data, const IntParameter& ip, int parameter_id)
    {
    if (ip.pointer)
      compile_int_parameter_pointer(code, data, ip, parameter_id);
    else
      compile_int_parameter_single(code, data, ip, parameter_id);
    }


  void compile_float_parameter_pointer(VM::vmcode& code, compile_data& data, const FloatParameter& fp, int parameter_id)
    {
    int64_t var_id = data.var_offset | variable_tag;
    data.var_offset += 8;
    if (parameter_id < 4)
      {
      VM::vmcode::operand op;
      switch (parameter_id)
        {
        case 0: op = CALLING_CONVENTION_INT_PAR_1; break;
        case 1: op = CALLING_CONVENTION_INT_PAR_2; break;
        case 2: op = CALLING_CONVENTION_INT_PAR_3; break;
        case 3: op = CALLING_CONVENTION_INT_PAR_4; break;
        default:
          throw_compile_error("calling convention error");
        }
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, op);
      }
    else
      {
      int addr = parameter_id - 4;
      code.add(VM::vmcode::MOV, VM::vmcode::RAX, VM::vmcode::MEM_RSP, rsp_offset + addr * 8 + virtual_machine_rsp_offset);
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::RAX);
      }
    auto it = data.vars.find(fp.name);
    if (it == data.vars.end())
      data.vars.insert(std::make_pair(fp.name, make_variable(var_id, external, pointer_to_single, memory_address)));
    else
      throw_compile_error(fp.line_nr, "Variable " + fp.name + " already exists");
    }

  void compile_float_parameter_single(VM::vmcode& code, compile_data& data, const FloatParameter& fp, int parameter_id)
    {
    auto it = data.vars.find(fp.name);
    if (it != data.vars.end())
      throw_compile_error(fp.line_nr, "Variable " + fp.name + " already exists");

    if (parameter_id < 4)
      {
      VM::vmcode::operand op;
      switch (parameter_id)
        {
        case 0: op = CALLING_CONVENTION_REAL_PAR_1; break;
        case 1: op = CALLING_CONVENTION_REAL_PAR_2; break;
        case 2: op = CALLING_CONVENTION_REAL_PAR_3; break;
        case 3: op = CALLING_CONVENTION_REAL_PAR_4; break;
        default:
          throw_compile_error("calling convention error");
        }
      assert(data.ra.is_free_real_register(op));
      data.vars.insert(std::make_pair(fp.name, make_variable(op, constant, single, register_address)));
      data.ra.make_real_register_unavailable(op);
      }
    else
      {
      int64_t var_id = data.var_offset | variable_tag;
      data.var_offset += 8;
      int addr = parameter_id - 4;
      code.add(VM::vmcode::MOV, VM::vmcode::RAX, VM::vmcode::MEM_RSP, rsp_offset + addr * 8 + virtual_machine_rsp_offset);
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::RAX);
      data.vars.insert(std::make_pair(fp.name, make_variable(var_id, constant, single, memory_address)));
      }
    }

  void compile_float_parameter(VM::vmcode& code, compile_data& data, const FloatParameter& fp, int parameter_id)
    {
    if (fp.pointer)
      compile_float_parameter_pointer(code, data, fp, parameter_id);
    else
      compile_float_parameter_single(code, data, fp, parameter_id);
    }
  void compile_parameters(VM::vmcode& code, compile_data& data, const Parameters& pars)
    {
    int calling_convention_id = 0;
    for (auto par : pars)
      {
      if (std::holds_alternative<IntParameter>(par))
        {
        compile_int_parameter(code, data, std::get<IntParameter>(par), calling_convention_id);
        ++calling_convention_id;
        }
      else if (std::holds_alternative<FloatParameter>(par))
        {
        compile_float_parameter(code, data, std::get<FloatParameter>(par), calling_convention_id);
        ++calling_convention_id;
        }
      else
        throw_compile_error("parameter type not implemented");
      }
    }

  } // anonymous namespace

void compile(VM::vmcode& code, const Program& prog)
  {
  compile_data data;

  code.add(VM::vmcode::SUB, VM::vmcode::RSP, VM::vmcode::NUMBER, rsp_offset);
  compile_parameters(code, data, prog.parameters);

  for (const auto& stm : prog.statements)
    compile_statement(code, data, stm);
  if (rt == RT_INTEGER)
    {
    code.add(VM::vmcode::CVTSI2SD, VM::vmcode::XMM0, VM::vmcode::RAX);
    }
  else if (FIRST_FREE_REAL_REG != VM::vmcode::XMM0)
    {
    code.add(VM::vmcode::MOV, VM::vmcode::XMM0, FIRST_FREE_REAL_REG);
    }
  code.add(VM::vmcode::ADD, VM::vmcode::RSP, VM::vmcode::NUMBER, rsp_offset);
  code.add(VM::vmcode::RET);

  offset_stack(code, data);
  }

COMPILER_END
