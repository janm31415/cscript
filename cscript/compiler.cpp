#include "compiler.h"

#include "utility.h"
#include "regalloc.h"
#include "defines.h"

#include <variant>
#include <functional>
#include <map>
#include <sstream>

COMPILER_BEGIN

namespace
  {

  std::vector<VM::vmcode::operand> get_external_calling_registers()
    {
    std::vector<VM::vmcode::operand> calling_registers;
    calling_registers.push_back(VM::vmcode::RCX);
    calling_registers.push_back(VM::vmcode::RDX);
    calling_registers.push_back(VM::vmcode::R8);
    calling_registers.push_back(VM::vmcode::R9);
    return calling_registers;
    }

  std::vector<VM::vmcode::operand> get_external_floating_point_registers()
    {
    std::vector<VM::vmcode::operand> calling_registers;
    calling_registers.push_back(VM::vmcode::XMM0);
    calling_registers.push_back(VM::vmcode::XMM1);
    calling_registers.push_back(VM::vmcode::XMM2);
    calling_registers.push_back(VM::vmcode::XMM3);
    return calling_registers;
    }

  std::vector<VM::vmcode::operand> get_registers_for_integer_variables()
    {
    std::vector<VM::vmcode::operand> reg;
    reg.push_back(CALLING_CONVENTION_INT_PAR_1);
    reg.push_back(CALLING_CONVENTION_INT_PAR_2);
    reg.push_back(CALLING_CONVENTION_INT_PAR_3);
    reg.push_back(CALLING_CONVENTION_INT_PAR_4);
    reg.push_back(REGISTER_FOR_INT_VARIABLE_1);
    reg.push_back(REGISTER_FOR_INT_VARIABLE_2);
    reg.push_back(REGISTER_FOR_INT_VARIABLE_3);
    reg.push_back(REGISTER_FOR_INT_VARIABLE_4);
    reg.push_back(REGISTER_FOR_INT_VARIABLE_5);
    reg.push_back(REGISTER_FOR_INT_VARIABLE_6);
    return reg;
    }

  std::vector<VM::vmcode::operand> get_registers_for_real_variables()
    {
    std::vector<VM::vmcode::operand> reg;
    reg.push_back(CALLING_CONVENTION_REAL_PAR_1);
    reg.push_back(CALLING_CONVENTION_REAL_PAR_2);
    reg.push_back(CALLING_CONVENTION_REAL_PAR_3);
    reg.push_back(CALLING_CONVENTION_REAL_PAR_4);
    reg.push_back(REGISTER_FOR_REAL_VARIABLE_1);
    reg.push_back(REGISTER_FOR_REAL_VARIABLE_2);
    reg.push_back(REGISTER_FOR_REAL_VARIABLE_3);
    reg.push_back(REGISTER_FOR_REAL_VARIABLE_4);
    reg.push_back(REGISTER_FOR_REAL_VARIABLE_5);
    reg.push_back(REGISTER_FOR_REAL_VARIABLE_6);
    reg.push_back(REGISTER_FOR_REAL_VARIABLE_7);
    reg.push_back(REGISTER_FOR_REAL_VARIABLE_8);
    reg.push_back(REGISTER_FOR_REAL_VARIABLE_9);
    return reg;
    }

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

  void compile_expression(VM::vmcode& code, compile_data& data, environment& env, const Expression& expr, const std::vector<external_function>& external_functions);
  void compile_statement(VM::vmcode& code, compile_data& data, environment& env, const Statement& stm, const std::vector<external_function>& external_functions);

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
      case 0: op = FIRST_TEMP_REAL_REG; offset = 0; break;
      case 1: op = SECOND_TEMP_REAL_REG; offset = 0; break;
      case 2: op = THIRD_TEMP_REAL_REG; offset = 0; break;
      default: op = VM::vmcode::MEM_RSP; offset = -(stack_index - 1) * 8; break;
      };
    }

  void index_to_integer_operand(VM::vmcode::operand& op, int64_t& offset, int64_t stack_index)
    {
    switch (stack_index)
      {
      case 0: op = FIRST_TEMP_INTEGER_REG; offset = 0; break;
      case 1: op = SECOND_TEMP_INTEGER_REG; offset = 0; break;
      case 2: op = THIRD_TEMP_INTEGER_REG; offset = 0; break;
      default: op = VM::vmcode::MEM_RSP; offset = -(stack_index - 1) * 8; break;
      };
    }

  void index_to_stack_operand(VM::vmcode::operand& op, int64_t& offset, int64_t stack_index)
    {
    op = VM::vmcode::MEM_RSP; offset = -(stack_index - 1) * 8;
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
      code.add(VM::vmcode::CVTTSD2SI, VM::vmcode::MEM_RSP, int_offset, VM::vmcode::MEM_RSP, real_offset);
      }
    else
      {
      if (int_offset)
        {
        code.add(VM::vmcode::CVTTSD2SI, VM::vmcode::MEM_RSP, int_offset, real_op);
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
      code.add(VM::vmcode::CMPEQPD, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      }
    else if (offset2)
      {
      code.add(VM::vmcode::CMPEQPD, op1, VM::vmcode::MEM_RSP, offset2);
      }
    else
      {
      code.add(VM::vmcode::CMPEQPD, op1, op2);
      }
    index_to_integer_operand(op2, offset2, data.stack_index - 1);
    if (offset1)
      {
      if (offset2)
        {
        code.add(VM::vmcode::MOVMSKPD, VM::vmcode::MEM_RSP, offset2, VM::vmcode::MEM_RSP, offset1);
        code.add(VM::vmcode::AND, VM::vmcode::MEM_RSP, offset2, VM::vmcode::NUMBER, 1);
        }
      else
        {
        code.add(VM::vmcode::MOVMSKPD, op2, VM::vmcode::MEM_RSP, offset1);
        code.add(VM::vmcode::AND, op2, VM::vmcode::NUMBER, 1);
        }
      }
    else
      {
      if (offset2)
        {
        code.add(VM::vmcode::MOVMSKPD, VM::vmcode::MEM_RSP, offset2, op1);
        code.add(VM::vmcode::AND, VM::vmcode::MEM_RSP, offset2, VM::vmcode::NUMBER, 1);
        }
      else
        {
        code.add(VM::vmcode::MOVMSKPD, op2, op1);
        code.add(VM::vmcode::AND, op2, VM::vmcode::NUMBER, 1);
        }
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
      code.add(VM::vmcode::SETE, VM::vmcode::MEM_RSP, offset1);
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
      code.add(VM::vmcode::CMPEQPD, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      }
    else if (offset2)
      {
      code.add(VM::vmcode::CMPEQPD, op1, VM::vmcode::MEM_RSP, offset2);
      }
    else
      {
      code.add(VM::vmcode::CMPEQPD, op1, op2);
      }
    index_to_integer_operand(op2, offset2, data.stack_index - 1);

    if (offset1)
      {
      if (offset2)
        {
        code.add(VM::vmcode::MOVMSKPD, VM::vmcode::MEM_RSP, offset2, VM::vmcode::MEM_RSP, offset1);
        code.add(VM::vmcode::AND, VM::vmcode::MEM_RSP, offset2, VM::vmcode::NUMBER, 1);
        code.add(VM::vmcode::XOR, VM::vmcode::MEM_RSP, offset2, VM::vmcode::NUMBER, 1);
        }
      else
        {
        code.add(VM::vmcode::MOVMSKPD, op2, VM::vmcode::MEM_RSP, offset1);
        code.add(VM::vmcode::AND, op2, VM::vmcode::NUMBER, 1);
        code.add(VM::vmcode::XOR, op2, VM::vmcode::NUMBER, 1);
        }
      }
    else
      {
      if (offset2)
        {
        code.add(VM::vmcode::MOVMSKPD, VM::vmcode::MEM_RSP, offset2, op1);
        code.add(VM::vmcode::AND, VM::vmcode::MEM_RSP, offset2, VM::vmcode::NUMBER, 1);
        code.add(VM::vmcode::XOR, VM::vmcode::MEM_RSP, offset2, VM::vmcode::NUMBER, 1);
        }
      else
        {
        code.add(VM::vmcode::MOVMSKPD, op2, op1);
        code.add(VM::vmcode::AND, op2, VM::vmcode::NUMBER, 1);
        code.add(VM::vmcode::XOR, op2, VM::vmcode::NUMBER, 1);
        }
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
      code.add(VM::vmcode::SETNE, VM::vmcode::MEM_RSP, offset1);
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
      code.add(VM::vmcode::CMPLTPD, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      }
    else if (offset2)
      {
      code.add(VM::vmcode::CMPLTPD, op1, VM::vmcode::MEM_RSP, offset2);
      }
    else
      {
      code.add(VM::vmcode::CMPLTPD, op1, op2);
      }
    index_to_integer_operand(op2, offset2, data.stack_index - 1);
    if (offset1)
      {
      if (offset2)
        {
        code.add(VM::vmcode::MOVMSKPD, VM::vmcode::MEM_RSP, offset2, VM::vmcode::MEM_RSP, offset1);
        code.add(VM::vmcode::AND, VM::vmcode::MEM_RSP, offset2, VM::vmcode::NUMBER, 1);
        }
      else
        {
        code.add(VM::vmcode::MOVMSKPD, op2, VM::vmcode::MEM_RSP, offset1);
        code.add(VM::vmcode::AND, op2, VM::vmcode::NUMBER, 1);
        }
      }
    else
      {
      if (offset2)
        {
        code.add(VM::vmcode::MOVMSKPD, VM::vmcode::MEM_RSP, offset2, op1);
        code.add(VM::vmcode::AND, VM::vmcode::MEM_RSP, offset2, VM::vmcode::NUMBER, 1);
        }
      else
        {
        code.add(VM::vmcode::MOVMSKPD, op2, op1);
        code.add(VM::vmcode::AND, op2, VM::vmcode::NUMBER, 1);
        }
      }
    rt = RT_INTEGER;

    /*
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
      code.add(VM::vmcode::MOVMSKPD, op1, op1);
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset2, op1);
      }
    else
      code.add(VM::vmcode::MOVMSKPD, op2, op1);

    rt = RT_INTEGER;
    */
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
      code.add(VM::vmcode::SETL, VM::vmcode::MEM_RSP, offset1);
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
      code.add(VM::vmcode::CMPLEPD, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      }
    else if (offset2)
      {
      code.add(VM::vmcode::CMPLEPD, op1, VM::vmcode::MEM_RSP, offset2);
      }
    else
      {
      code.add(VM::vmcode::CMPLEPD, op1, op2);
      }
    index_to_integer_operand(op2, offset2, data.stack_index - 1);
    if (offset1)
      {
      if (offset2)
        {
        code.add(VM::vmcode::MOVMSKPD, VM::vmcode::MEM_RSP, offset2, VM::vmcode::MEM_RSP, offset1);
        code.add(VM::vmcode::AND, VM::vmcode::MEM_RSP, offset2, VM::vmcode::NUMBER, 1);
        }
      else
        {
        code.add(VM::vmcode::MOVMSKPD, op2, VM::vmcode::MEM_RSP, offset1);
        code.add(VM::vmcode::AND, op2, VM::vmcode::NUMBER, 1);
        }
      }
    else
      {
      if (offset2)
        {
        code.add(VM::vmcode::MOVMSKPD, VM::vmcode::MEM_RSP, offset2, op1);
        code.add(VM::vmcode::AND, VM::vmcode::MEM_RSP, offset2, VM::vmcode::NUMBER, 1);
        }
      else
        {
        code.add(VM::vmcode::MOVMSKPD, op2, op1);
        code.add(VM::vmcode::AND, op2, VM::vmcode::NUMBER, 1);
        }
      }
    rt = RT_INTEGER;
    /*
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
      code.add(VM::vmcode::MOVMSKPD, VM::vmcode::MEM_RSP, offset2, op1);
      code.add(VM::vmcode::AND, VM::vmcode::MEM_RSP, offset2, VM::vmcode::NUMBER, 1);
      }
    else
      {
      code.add(VM::vmcode::MOVMSKPD, op2, op1);
      code.add(VM::vmcode::AND, op2, VM::vmcode::NUMBER, 1);
      }

    rt = RT_INTEGER;
    */
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
      code.add(VM::vmcode::SETLE, VM::vmcode::MEM_RSP, offset1);
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
      code.add(VM::vmcode::CMPLEPD, VM::vmcode::MEM_RSP, offset2, VM::vmcode::MEM_RSP, offset1);
      }
    else if (offset2)
      {
      code.add(VM::vmcode::CMPLEPD, VM::vmcode::MEM_RSP, offset2, op1);
      }
    else
      {
      code.add(VM::vmcode::CMPLEPD, op2, op1);
      }
    index_to_integer_operand(op1, offset1, data.stack_index - 1);
    if (offset2)
      {
      if (offset1)
        {
        code.add(VM::vmcode::MOVMSKPD, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
        code.add(VM::vmcode::AND, VM::vmcode::MEM_RSP, offset1, VM::vmcode::NUMBER, 1);
        }
      else
        {
        code.add(VM::vmcode::MOVMSKPD, op1, VM::vmcode::MEM_RSP, offset2);
        code.add(VM::vmcode::AND, op1, VM::vmcode::NUMBER, 1);
        }
      }
    else
      {
      if (offset2)
        {
        code.add(VM::vmcode::MOVMSKPD, VM::vmcode::MEM_RSP, offset1, op2);
        code.add(VM::vmcode::AND, VM::vmcode::MEM_RSP, offset1, VM::vmcode::NUMBER, 1);
        }
      else
        {
        code.add(VM::vmcode::MOVMSKPD, op1, op2);
        code.add(VM::vmcode::AND, op1, VM::vmcode::NUMBER, 1);
        }
      }
    rt = RT_INTEGER;


    /*
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
      code.add(VM::vmcode::MOVMSKPD, VM::vmcode::MEM_RSP, offset1, op2);
      }
    else
      {
      code.add(VM::vmcode::MOVMSKPD, op1, op2);

      }
    rt = RT_INTEGER;
    */
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
      code.add(VM::vmcode::SETG, VM::vmcode::MEM_RSP, offset1);
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
      code.add(VM::vmcode::CMPLTPD, VM::vmcode::MEM_RSP, offset2, VM::vmcode::MEM_RSP, offset1);
      }
    else if (offset2)
      {
      code.add(VM::vmcode::CMPLTPD, VM::vmcode::MEM_RSP, offset2, op1);
      }
    else
      {
      code.add(VM::vmcode::CMPLTPD, op2, op1);
      }
    index_to_integer_operand(op1, offset1, data.stack_index - 1);
    if (offset2)
      {
      if (offset1)
        {
        code.add(VM::vmcode::MOVMSKPD, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
        code.add(VM::vmcode::AND, VM::vmcode::MEM_RSP, offset1, VM::vmcode::NUMBER, 1);
        }
      else
        {
        code.add(VM::vmcode::MOVMSKPD, op1, VM::vmcode::MEM_RSP, offset2);
        code.add(VM::vmcode::AND, op1, VM::vmcode::NUMBER, 1);
        }
      }
    else
      {
      if (offset2)
        {
        code.add(VM::vmcode::MOVMSKPD, VM::vmcode::MEM_RSP, offset1, op2);
        code.add(VM::vmcode::AND, VM::vmcode::MEM_RSP, offset1, VM::vmcode::NUMBER, 1);
        }
      else
        {
        code.add(VM::vmcode::MOVMSKPD, op1, op2);
        code.add(VM::vmcode::AND, op1, VM::vmcode::NUMBER, 1);
        }
      }
    rt = RT_INTEGER;

    /*
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
      code.add(VM::vmcode::MOVMSKPD, VM::vmcode::MEM_RSP, offset1, op2);
      code.add(VM::vmcode::AND, VM::vmcode::MEM_RSP, offset1, VM::vmcode::NUMBER, 1);
      }
    else
      {
      code.add(VM::vmcode::MOVMSKPD, op1, op2);
      code.add(VM::vmcode::AND, op1, VM::vmcode::NUMBER, 1);
      }
    rt = RT_INTEGER;
    */
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
      code.add(VM::vmcode::SETGE, VM::vmcode::MEM_RSP, offset1);
      }
    else
      {
      code.add(VM::vmcode::SETGE, op1);
      }
    rt = RT_INTEGER;
    }

  void compile_mod(VM::vmcode& code, compile_data& data)
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
        code.add(VM::vmcode::MODSD, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::MODSD, VM::vmcode::MEM_RSP, offset1, op2);
      }
    else
      {
      if (offset2)
        code.add(VM::vmcode::MODSD, op1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::MODSD, op1, op2);
      }
    rt = RT_REAL;
    }

  void compile_modi(VM::vmcode& code, compile_data& data)
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
        code.add(VM::vmcode::MOD, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::MOD, VM::vmcode::MEM_RSP, offset1, op2);
      }
    else
      {
      if (offset2)
        code.add(VM::vmcode::MOD, op1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::MOD, op1, op2);
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
    rt = RT_INTEGER;
    }

  void change_sign_real(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_real_operand(op1, offset1, data.stack_index);
    if (offset1)
      {
      code.add(VM::vmcode::XORPD, VM::vmcode::MEM_RSP, offset1, VM::vmcode::NUMBER, 0x8000000000000000);
      }
    else
      code.add(VM::vmcode::XORPD, op1, VM::vmcode::NUMBER, 0x8000000000000000);
    rt = RT_REAL;
    }

  void change_sign_integer(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_integer_operand(op1, offset1, data.stack_index);
    if (offset1)
      {
      code.add(VM::vmcode::NEG, VM::vmcode::MEM_RSP, offset1);
      }
    else
      code.add(VM::vmcode::NEG, op1);
    rt = RT_INTEGER;
    }

  void compile_sqrt(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op;
    int64_t offset;
    index_to_real_operand(op, offset, data.stack_index);
    if (offset)
      code.add(VM::vmcode::SQRTPD, VM::vmcode::MEM_RSP, offset, VM::vmcode::MEM_RSP, offset);
    else
      code.add(VM::vmcode::SQRTPD, op, op);
    rt = RT_REAL;
    }

  void compile_sin(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op;
    int64_t offset;
    index_to_real_operand(op, offset, data.stack_index);
    if (offset)
      code.add(VM::vmcode::CSIN, VM::vmcode::MEM_RSP, offset, VM::vmcode::MEM_RSP, offset);
    else
      code.add(VM::vmcode::CSIN, op, op);
    rt = RT_REAL;
    }

  void compile_cos(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op;
    int64_t offset;
    index_to_real_operand(op, offset, data.stack_index);
    if (offset)
      code.add(VM::vmcode::CCOS, VM::vmcode::MEM_RSP, offset, VM::vmcode::MEM_RSP, offset);
    else
      code.add(VM::vmcode::CCOS, op, op);
    rt = RT_REAL;
    }

  void compile_exp(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op;
    int64_t offset;
    index_to_real_operand(op, offset, data.stack_index);
    if (offset)
      code.add(VM::vmcode::CEXP, VM::vmcode::MEM_RSP, offset, VM::vmcode::MEM_RSP, offset);
    else
      code.add(VM::vmcode::CEXP, op, op);
    rt = RT_REAL;
    }

  void compile_log(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op;
    int64_t offset;
    index_to_real_operand(op, offset, data.stack_index);
    if (offset)
      code.add(VM::vmcode::CLOG, VM::vmcode::MEM_RSP, offset, VM::vmcode::MEM_RSP, offset);
    else
      code.add(VM::vmcode::CLOG, op, op);
    rt = RT_REAL;
    }

  void compile_log2(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op;
    int64_t offset;
    index_to_real_operand(op, offset, data.stack_index);
    if (offset)
      code.add(VM::vmcode::CLOG2, VM::vmcode::MEM_RSP, offset, VM::vmcode::MEM_RSP, offset);
    else
      code.add(VM::vmcode::CLOG2, op, op);
    rt = RT_REAL;
    }

  void compile_fabs(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op;
    int64_t offset;
    index_to_real_operand(op, offset, data.stack_index);
    if (offset)
      code.add(VM::vmcode::CABS, VM::vmcode::MEM_RSP, offset, VM::vmcode::MEM_RSP, offset);
    else
      code.add(VM::vmcode::CABS, op, op);
    rt = RT_REAL;
    }

  void compile_tan(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op;
    int64_t offset;
    index_to_real_operand(op, offset, data.stack_index);
    if (offset)
      code.add(VM::vmcode::CTAN, VM::vmcode::MEM_RSP, offset, VM::vmcode::MEM_RSP, offset);
    else
      code.add(VM::vmcode::CTAN, op, op);
    rt = RT_REAL;
    }

  void compile_atan(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op;
    int64_t offset;
    index_to_real_operand(op, offset, data.stack_index);
    if (offset)
      code.add(VM::vmcode::CATAN, VM::vmcode::MEM_RSP, offset, VM::vmcode::MEM_RSP, offset);
    else
      code.add(VM::vmcode::CATAN, op, op);
    rt = RT_REAL;
    }

  void compile_atan2(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_real_operand(op1, offset1, data.stack_index);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_real_operand(op2, offset2, data.stack_index + 1);
    if (offset1)
      {
      if (offset2)
        code.add(VM::vmcode::CATAN2, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::CATAN2, VM::vmcode::MEM_RSP, offset1, op2);
      }
    else
      {
      if (offset2)
        code.add(VM::vmcode::CATAN2, op1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::CATAN2, op1, op2);
      }
    rt = RT_REAL;
    }

  void compile_pow(VM::vmcode& code, compile_data& data)
    {
    VM::vmcode::operand op1;
    int64_t offset1;
    index_to_real_operand(op1, offset1, data.stack_index);
    VM::vmcode::operand op2;
    int64_t offset2;
    index_to_real_operand(op2, offset2, data.stack_index + 1);
    if (offset1)
      {
      if (offset2)
        code.add(VM::vmcode::CPOW, VM::vmcode::MEM_RSP, offset1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::CPOW, VM::vmcode::MEM_RSP, offset1, op2);
      }
    else
      {
      if (offset2)
        code.add(VM::vmcode::CPOW, op1, VM::vmcode::MEM_RSP, offset2);
      else
        code.add(VM::vmcode::CPOW, op1, op2);
      }
    rt = RT_REAL;
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
      {"%", compile_mod},
      {"%i", compile_modi},
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
      {"sin", compile_sin},
      {"cos", compile_cos},
      {"exp", compile_exp},
      {"log", compile_log},
      {"log2", compile_log2},
      {"fabs", compile_fabs},
      {"tan", compile_tan},
      {"atan", compile_atan},
      {"atan2", compile_atan2},
      {"pow", compile_pow}
    };

  void compile_make_int_array(VM::vmcode& code, compile_data& data, environment& /*env*/, const Int& make_i)
    {
    if (make_i.dims.size() != 1)
      throw_compile_error(make_i.line_nr, "compile error: only single dimension arrays are allowed");
    if (!is_constant(make_i.dims.front()))
      throw_compile_error(make_i.line_nr, "compile error: array dimension should be a constant");
    int64_t val = to_i(get_constant_value(make_i.dims.front()).front());
    if (val <= 0)
      throw_compile_error(make_i.line_nr, "compile error: array dimension should be a non zero positive number");
    values init_values;
    bool init = !make_i.expr.operands.empty();
    if (init)
      {
      if (make_i.expr.operands.size() != 1)
        throw_compile_error(make_i.line_nr, "compile error: array initialization expects an expression list");
      if (!is_constant(make_i.expr.operands.front()))
        throw_compile_error(make_i.line_nr, "compile error: array initialization expects a constant expression list");
      init_values = get_constant_value(make_i.expr);
      if ((int64_t)init_values.size() != val)
        throw_compile_error(make_i.line_nr, "compile error: array initializer list has wrong dimension");
      }
    auto it = data.vars.find(make_i.name);
    if (it == data.vars.end())
      {
      int64_t var_id = (data.var_offset + 8 * (val -1)) | variable_tag;
      data.vars.insert(std::make_pair(make_i.name, make_variable(var_id, constant, integer_array, memory_address)));

      if (init)
        {
        for (const auto& iv : init_values)
          {
          uint64_t val64 = 0;
          if (std::holds_alternative<double>(iv))
            val64 = *(uint64_t*)(&std::get<double>(iv));
          else
            val64 = (uint64_t)std::get<int64_t>(iv);
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::NUMBER, val64);
          var_id -= 8;
          }
        }

      data.var_offset += 8 * val;
      }
    else
      {
      throw_compile_error(make_i.line_nr, "Variable " + make_i.name + " already exists");
      }
    }

  void compile_make_global_int_single(VM::vmcode& code, compile_data& data, environment& env, const Int& make_i, const std::vector<external_function>& external_functions)
    {
    bool init = !make_i.expr.operands.empty();
    VM::vmcode::operand int_op;
    int64_t int_offset = 0;
    if (init)
      {
      compile_expression(code, data, env, make_i.expr, external_functions);
      if (rt == RT_REAL)
        {
        convert_real_to_integer(code, data.stack_index);
        }
      rt = RT_INTEGER;
      index_to_integer_operand(int_op, int_offset, data.stack_index);
      }
    auto it = env.globals.find(make_i.name);
    if (it == env.globals.end())
      {
      global_variable_type new_global;
      new_global.address = env.global_var_offset;
      new_global.vt = global_value_integer;
      env.globals.insert(std::make_pair(make_i.name, new_global));
      if (init)
        {
        if (int_offset)
          code.add(VM::vmcode::MOV, GLOBAL_VARIABLE_MEM_REG, env.global_var_offset, VM::vmcode::MEM_RSP, int_offset);
        else
          code.add(VM::vmcode::MOV, GLOBAL_VARIABLE_MEM_REG, env.global_var_offset, int_op);
        }
      env.global_var_offset += 8;
      }
    else
      {
      throw_compile_error(make_i.line_nr, "Global variable " + make_i.name + " already exists");
      }
    }

  void compile_make_int_single(VM::vmcode& code, compile_data& data, environment& env, const Int& make_i, const std::vector<external_function>& external_functions)
    {
    bool init = !make_i.expr.operands.empty();
    VM::vmcode::operand int_op;
    int64_t int_offset = 0;
    if (init)
      {
      compile_expression(code, data, env, make_i.expr, external_functions);
      if (rt == RT_REAL)
        {
        convert_real_to_integer(code, data.stack_index);
        }
      rt = RT_INTEGER;
      index_to_integer_operand(int_op, int_offset, data.stack_index);
      }
    auto it = data.vars.find(make_i.name);
    if (it == data.vars.end())
      {
      if (data.ra.free_integer_register_available())
        {
        auto reg = data.ra.get_next_available_integer_register();
        data.ra.make_integer_register_unavailable(reg);
        if (init)
          {
          if (int_offset)
            code.add(VM::vmcode::MOV, reg, VM::vmcode::MEM_RSP, int_offset);
          else
            code.add(VM::vmcode::MOV, reg, int_op);
          }
        data.vars.insert(std::make_pair(make_i.name, make_variable(reg, constant, integer, register_address)));
        }
      else
        {
        int64_t var_id = data.var_offset | variable_tag;
        if (init)
          {
          if (int_offset)
            code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::MEM_RSP, int_offset);
          else
            code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, int_op);
          }
        data.vars.insert(std::make_pair(make_i.name, make_variable(var_id, constant, integer, memory_address)));
        data.var_offset += 8;
        }
      }
    else
      {
      throw_compile_error(make_i.line_nr, "Variable " + make_i.name + " already exists");
      }
    }

  void compile_make_int(VM::vmcode& code, compile_data& data, environment& env, const Int& make_i, const std::vector<external_function>& external_functions)
    {
    if (!make_i.dims.empty())
      {
      if (make_i.name.front() == '$')
        throw_compile_error(make_i.line_nr, "Global variables can't be arrays");
      else
        compile_make_int_array(code, data, env, make_i);
      }
    else
      {
      if (make_i.name.front() == '$')
        compile_make_global_int_single(code, data, env, make_i, external_functions);
      else
        compile_make_int_single(code, data, env, make_i, external_functions);
      }
    }


  void compile_make_float_array(VM::vmcode& code, compile_data& data, environment& /*env*/, const Float& make_f)
    {
    if (make_f.dims.size() != 1)
      throw_compile_error(make_f.line_nr, "compile error: only single dimension arrays are allowed");
    if (!is_constant(make_f.dims.front()))
      throw_compile_error(make_f.line_nr, "compile error: array dimension should be a constant");
    int64_t val = to_i(get_constant_value(make_f.dims.front()).front());
    if (val <= 0)
      throw_compile_error(make_f.line_nr, "compile error: array dimension should be a non zero positive number");
    values init_values;
    bool init = !make_f.expr.operands.empty();
    if (init)
      {
      if (make_f.expr.operands.size() != 1)
        throw_compile_error(make_f.line_nr, "compile error: array initialization expects an expression list");
      if (!is_constant(make_f.expr.operands.front()))
        throw_compile_error(make_f.line_nr, "compile error: array initialization expects a constant expression list");
      init_values = get_constant_value(make_f.expr);
      if ((int64_t)init_values.size() != val)
        throw_compile_error(make_f.line_nr, "compile error: array initializer list has wrong dimension");
      }
    auto it = data.vars.find(make_f.name);
    if (it == data.vars.end())
      {
      int64_t var_id = (data.var_offset + 8*(val-1)) | variable_tag;
      data.vars.insert(std::make_pair(make_f.name, make_variable(var_id, constant, real_array, memory_address)));

      if (init)
        {
        for (const auto& iv : init_values)
          {
          uint64_t val64 = 0;
          if (std::holds_alternative<double>(iv))
            val64 = *(uint64_t*)(&std::get<double>(iv));
          else
            val64 = (uint64_t)std::get<int64_t>(iv);
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::NUMBER, val64);
          var_id -= 8;
          }
        }

      data.var_offset += 8 * val;
      }
    else
      {
      throw_compile_error(make_f.line_nr, "Variable " + make_f.name + " already exists");
      }
    }

  void compile_make_global_float_real(VM::vmcode& code, compile_data& data, environment& env, const Float& make_f, const std::vector<external_function>& external_functions)
    {
    bool init = !make_f.expr.operands.empty();
    VM::vmcode::operand int_op;
    int64_t int_offset = 0;
    if (init)
      {
      compile_expression(code, data, env, make_f.expr, external_functions);
      if (rt == RT_INTEGER)
        {
        convert_integer_to_real(code, data.stack_index);
        }
      rt = RT_REAL;
      index_to_real_operand(int_op, int_offset, data.stack_index);
      }
    auto it = env.globals.find(make_f.name);
    if (it == env.globals.end())
      {
      global_variable_type new_global;
      new_global.address = env.global_var_offset;
      new_global.vt = global_value_real;
      env.globals.insert(std::make_pair(make_f.name, new_global));
      if (init)
        {
        if (int_offset)
          code.add(VM::vmcode::MOV, GLOBAL_VARIABLE_MEM_REG, env.global_var_offset, VM::vmcode::MEM_RSP, int_offset);
        else
          code.add(VM::vmcode::MOV, GLOBAL_VARIABLE_MEM_REG, env.global_var_offset, int_op);
        }
      env.global_var_offset += 8;
      }
    else
      {
      throw_compile_error(make_f.line_nr, "Global variable " + make_f.name + " already exists");
      }
    }

  void compile_make_float_single(VM::vmcode& code, compile_data& data, environment& env, const Float& make_f, const std::vector<external_function>& external_functions)
    {
    bool init = !make_f.expr.operands.empty();
    VM::vmcode::operand float_op;
    int64_t float_offset = 0;
    if (init)
      {
      compile_expression(code, data, env, make_f.expr, external_functions);
      if (rt == RT_INTEGER)
        {
        convert_integer_to_real(code, data.stack_index);
        }

      rt = RT_REAL;
      index_to_real_operand(float_op, float_offset, data.stack_index);
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
            code.add(VM::vmcode::MOV, reg, VM::vmcode::MEM_RSP, float_offset);
          else
            code.add(VM::vmcode::MOV, reg, float_op);
          }
        data.vars.insert(std::make_pair(make_f.name, make_variable(reg, constant, real, register_address)));
        }
      else
        {
        int64_t var_id = data.var_offset | variable_tag;
        if (init)
          {
          if (float_offset)
            code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::MEM_RSP, float_offset);
          else
            code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, float_op);
          }
        data.vars.insert(std::make_pair(make_f.name, make_variable(var_id, constant, real, memory_address)));
        data.var_offset += 8;
        }
      }
    else
      {
      throw_compile_error(make_f.line_nr, "Variable " + make_f.name + " already exists");
      }
    }

  void compile_make_float(VM::vmcode& code, compile_data& data, environment& env, const Float& make_f, const std::vector<external_function>& external_functions)
    {
    if (!make_f.dims.empty())
      {
      if (make_f.name.front() == '$')
        throw_compile_error(make_f.line_nr, "Global variables can't be arrays");
      else
        compile_make_float_array(code, data, env, make_f);
      }
    else
      {
      if (make_f.name.front() == '$')
        compile_make_global_float_real(code, data, env, make_f, external_functions);
      else
        compile_make_float_single(code, data, env, make_f, external_functions);
      }
    }

  void compile_local_variable(VM::vmcode& code, compile_data& data, environment& /*env*/, const Variable& v)
    {
    auto it = data.vars.find(v.name);
    if (it == data.vars.end())
      throw_compile_error(v.line_nr, "Cannot find variable " + v.name);
    int64_t var_id = it->second.address;
    bool is_array = it->second.vt == real_array || it->second.vt == integer_array;

    if (is_array) // get address, not value
      {
      rt = RT_INTEGER;
      assert(it->second.st == constant); // external parameters cannot be arrays currently
      assert(it->second.at == memory_address); // arrays live on the stack
      VM::vmcode::operand op;
      int64_t offset;
      index_to_integer_operand(op, offset, data.stack_index);
      //code.add(VM::vmcode::MOV, SECOND_TEMP_INTEGER_REG, VM::vmcode::NUMBER, var_id & 0xFFFFFFFFFFFFFFF8);
      //code.add(VM::vmcode::SHL, SECOND_TEMP_INTEGER_REG, VM::vmcode::NUMBER, 3);
      code.add(VM::vmcode::MOV, FIRST_TEMP_INTEGER_REG, VM::vmcode::RSP);
      code.add(VM::vmcode::ADD, FIRST_TEMP_INTEGER_REG, VM::vmcode::NUMBER, (-var_id & 0xFFFFFFFFFFFFFFF8));

      if (offset)
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, FIRST_TEMP_INTEGER_REG);
      else
        code.add(VM::vmcode::MOV, op, FIRST_TEMP_INTEGER_REG);
      }
    else
      {
      rt = (it->second.vt == real) ? RT_REAL : RT_INTEGER;
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
              code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, int_offset, VM::vmcode::MEM_RSP, -var_id);
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
              code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, real_offset, VM::vmcode::MEM_RSP, -var_id);
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
    }

  void compile_global_variable(VM::vmcode& code, compile_data& data, environment& env, const Variable& v)
    {
    auto it = env.globals.find(v.name);
    if (it == env.globals.end())
      throw_compile_error(v.line_nr, "Cannot find global variable " + v.name);
    int64_t address = it->second.address;
    rt = (it->second.vt == global_value_real) ? RT_REAL : RT_INTEGER;
    if (rt == RT_INTEGER)
      {
      VM::vmcode::operand int_op;
      int64_t int_offset;
      index_to_integer_operand(int_op, int_offset, data.stack_index);
      if (int_offset)
        {
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, int_offset, GLOBAL_VARIABLE_MEM_REG, address);
        }
      else
        {
        code.add(VM::vmcode::MOV, int_op, GLOBAL_VARIABLE_MEM_REG, address);
        }
      }
    else
      {
      VM::vmcode::operand real_op;
      int64_t real_offset;
      index_to_real_operand(real_op, real_offset, data.stack_index);
      if (real_offset)
        {
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, real_offset, GLOBAL_VARIABLE_MEM_REG, address);
        }
      else
        {
        code.add(VM::vmcode::MOV, real_op, GLOBAL_VARIABLE_MEM_REG, address);
        }
      }
    }

  void compile_variable(VM::vmcode& code, compile_data& data, environment& env, const Variable& v)
    {
    if (v.name.front() == '$')
      compile_global_variable(code, data, env, v);
    else
      compile_local_variable(code, data, env, v);
    }

  void compile_assignment_pointer(VM::vmcode& code, compile_data& data, environment& env, const Assignment& a, const std::vector<external_function>& external_functions)
    {
    if (a.dims.size() != 1)
      throw_compile_error(a.line_nr, "only single dimension arrays are allowed");
    if (data.stack_index != 0)
      throw_compile_error(a.line_nr, "assignment only as single statement allowed");
    compile_expression(code, data, env, a.dims.front(), external_functions);
    if (rt == RT_REAL)
      {
      convert_real_to_integer(code, data.stack_index);
      rt = RT_INTEGER;
      }
    code.add(VM::vmcode::SHL, FIRST_TEMP_INTEGER_REG, VM::vmcode::NUMBER, 3);
    ++data.stack_index;
    update_data(data);
    compile_expression(code, data, env, a.expr, external_functions);
    auto it = data.vars.find(a.name);
    if (it == data.vars.end())
      throw_compile_error(a.line_nr, "variable " + a.name + " is not declared.");
    //if (it->second.st != constant)
    //  throw_compile_error(a.line_nr, "variable " + a.name + " cannot be assigned.");
    int64_t var_id = it->second.address;
    if (it->second.vt != pointer_to_real && it->second.vt != pointer_to_integer)
      throw_compile_error(a.line_nr, "I expect " + a.name + " to be a pointer.");
    if (it->second.vt == pointer_to_real && rt == RT_INTEGER)
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
    code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, FIRST_TEMP_INTEGER_REG);
    //r10 contains the pointer address
    //SECOND_TEMP_REAL_REG or SECOND_TEMP_INTEGER_REG contains the expression value
    switch (a.op.front())
      {
      case '=':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_REAL_REG);
        }
      else
        {
        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_INTEGER_REG);
        }
      break;
      }
      case '+':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::ADDSD, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_REAL_REG);
        }
      else
        {
        code.add(VM::vmcode::ADD, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_INTEGER_REG);
        }
      break;
      }
      case '-':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::SUBSD, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_REAL_REG);
        }
      else
        {
        code.add(VM::vmcode::SUB, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_INTEGER_REG);
        }
      break;
      }
      case '*':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::MULSD, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_REAL_REG);
        }
      else
        {
        code.add(VM::vmcode::IMUL, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_INTEGER_REG);
        }
      break;
      }
      case '/':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::DIVSD, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_REAL_REG);
        }
      else
        {
        code.add(VM::vmcode::IDIV2, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_INTEGER_REG);
        }
      break;
      }
      default: throw std::runtime_error("not implemented");
      };
    }

  void compile_assignment_array(VM::vmcode& code, compile_data& data, environment& env, const Assignment& a, const std::vector<external_function>& external_functions)
    {
    if (a.dims.size() != 1)
      throw_compile_error(a.line_nr, "only single dimension arrays are allowed");
    if (data.stack_index != 0)
      throw_compile_error(a.line_nr, "assignment only as single statement allowed");
    compile_expression(code, data, env, a.dims.front(), external_functions);
    if (rt == RT_REAL)
      {
      convert_real_to_integer(code, data.stack_index);
      rt = RT_INTEGER;
      }
    code.add(VM::vmcode::SHL, FIRST_TEMP_INTEGER_REG, VM::vmcode::NUMBER, 3);
    ++data.stack_index;
    update_data(data);
    compile_expression(code, data, env, a.expr, external_functions);
    auto it = data.vars.find(a.name);
    if (it == data.vars.end())
      throw_compile_error(a.line_nr, "variable " + a.name + " is not declared.");
    if (it->second.st != constant)
      throw_compile_error(a.line_nr, "variable " + a.name + " cannot be assigned.");
    int64_t var_id = it->second.address;
    if (it->second.vt != real_array && it->second.vt != integer_array)
      throw_compile_error(a.line_nr, "I expect " + a.name + " to be array.");
    if (it->second.at != memory_address)
      throw_compile_error(a.line_nr, "I expect " + a.name + " to be an array on the stack.");
    if (it->second.vt == real_array && rt == RT_INTEGER)
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
    //FIRST_TEMP_INTEGER_REG contains the index
    //SECOND_TEMP_REAL_REG or SECOND_TEMP_INTEGER_REG contains the expression value
    switch (a.op.front())
      {
      case '=':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::ADD, VM::vmcode::RSP, FIRST_TEMP_INTEGER_REG);
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, SECOND_TEMP_REAL_REG);
        code.add(VM::vmcode::SUB, VM::vmcode::RSP, FIRST_TEMP_INTEGER_REG);
        }
      else
        {
        code.add(VM::vmcode::ADD, VM::vmcode::RSP, FIRST_TEMP_INTEGER_REG);
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, SECOND_TEMP_INTEGER_REG);
        code.add(VM::vmcode::SUB, VM::vmcode::RSP, FIRST_TEMP_INTEGER_REG);
        }
      break;
      }
      case '+':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::ADD, VM::vmcode::RSP, FIRST_TEMP_INTEGER_REG);
        code.add(VM::vmcode::ADDSD, VM::vmcode::MEM_RSP, -var_id, SECOND_TEMP_REAL_REG);
        code.add(VM::vmcode::SUB, VM::vmcode::RSP, FIRST_TEMP_INTEGER_REG);
        }
      else
        {
        code.add(VM::vmcode::ADD, VM::vmcode::RSP, FIRST_TEMP_INTEGER_REG);
        code.add(VM::vmcode::ADD, VM::vmcode::MEM_RSP, -var_id, SECOND_TEMP_INTEGER_REG);
        code.add(VM::vmcode::SUB, VM::vmcode::RSP, FIRST_TEMP_INTEGER_REG);
        }
      break;
      }
      case '-':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::ADD, VM::vmcode::RSP, FIRST_TEMP_INTEGER_REG);
        code.add(VM::vmcode::SUBSD, VM::vmcode::MEM_RSP, -var_id, SECOND_TEMP_REAL_REG);
        code.add(VM::vmcode::SUB, VM::vmcode::RSP, FIRST_TEMP_INTEGER_REG);
        }
      else
        {
        code.add(VM::vmcode::ADD, VM::vmcode::RSP, FIRST_TEMP_INTEGER_REG);
        code.add(VM::vmcode::SUB, VM::vmcode::MEM_RSP, -var_id, SECOND_TEMP_INTEGER_REG);
        code.add(VM::vmcode::SUB, VM::vmcode::RSP, FIRST_TEMP_INTEGER_REG);
        }
      break;
      }
      case '*':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::ADD, VM::vmcode::RSP, FIRST_TEMP_INTEGER_REG);
        code.add(VM::vmcode::MULSD, VM::vmcode::MEM_RSP, -var_id, SECOND_TEMP_REAL_REG);
        code.add(VM::vmcode::SUB, VM::vmcode::RSP, FIRST_TEMP_INTEGER_REG);
        }
      else
        {
        code.add(VM::vmcode::ADD, VM::vmcode::RSP, FIRST_TEMP_INTEGER_REG);
        code.add(VM::vmcode::IMUL, SECOND_TEMP_INTEGER_REG, VM::vmcode::MEM_RSP, -var_id);
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, SECOND_TEMP_INTEGER_REG);
        code.add(VM::vmcode::SUB, VM::vmcode::RSP, FIRST_TEMP_INTEGER_REG);
        }
      break;
      }
      case '/':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::ADD, VM::vmcode::RSP, FIRST_TEMP_INTEGER_REG);
        code.add(VM::vmcode::DIVSD, VM::vmcode::MEM_RSP, -var_id, SECOND_TEMP_REAL_REG);
        code.add(VM::vmcode::SUB, VM::vmcode::RSP, FIRST_TEMP_INTEGER_REG);
        }
      else
        {
        code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::RSP);
        code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, FIRST_TEMP_INTEGER_REG);
        code.add(VM::vmcode::IDIV2, STACK_MEM_BACKUP_REGISTER, -var_id, SECOND_TEMP_INTEGER_REG);
        }
      break;
      }
      default: throw std::runtime_error("not implemented");
      };
    }

  void compile_assignment_dereference(VM::vmcode& code, compile_data& data, environment& env, const Assignment& a, const std::vector<external_function>& external_functions)
    {
    if (!a.dims.empty())
      throw_compile_error(a.line_nr, "dereference without dimensions expected");
    if (data.stack_index != 0)
      throw_compile_error(a.line_nr, "assignment only as single statement allowed");
    ++data.stack_index;
    update_data(data);
    compile_expression(code, data, env, a.expr, external_functions);
    auto it = data.vars.find(a.name);
    if (it == data.vars.end())
      throw_compile_error(a.line_nr, "variable " + a.name + " is not declared.");
    int64_t var_id = it->second.address;
    if (it->second.vt != pointer_to_real && it->second.vt != pointer_to_integer)
      throw_compile_error(a.line_nr, "I expect " + a.name + " to be a pointer.");
    if (it->second.vt == pointer_to_real && rt == RT_INTEGER)
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
    //SECOND_TEMP_REAL_REG or SECOND_TEMP_INTEGER_REG contains the expression value
    switch (a.op.front())
      {
      case '=':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_REAL_REG);
        }
      else
        {
        code.add(VM::vmcode::MOV, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_INTEGER_REG);
        }
      break;
      }
      case '+':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::ADDSD, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_REAL_REG);
        }
      else
        {
        code.add(VM::vmcode::ADD, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_INTEGER_REG);
        }
      break;
      }
      case '-':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::SUBSD, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_REAL_REG);
        }
      else
        {
        code.add(VM::vmcode::SUB, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_INTEGER_REG);
        }
      break;
      }
      case '*':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::MULSD, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_REAL_REG);
        }
      else
        {
        code.add(VM::vmcode::IMUL, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_INTEGER_REG);
        }
      break;
      }
      case '/':
      {
      if (rt == RT_REAL)
        {
        code.add(VM::vmcode::DIVSD, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_REAL_REG);
        }
      else
        {
        code.add(VM::vmcode::IDIV2, STACK_MEM_BACKUP_REGISTER, SECOND_TEMP_INTEGER_REG);
        }
      break;
      }
      default: throw std::runtime_error("not implemented");
      };
    }

  void compile_assignment_single(VM::vmcode& code, compile_data& data, environment& env, const Assignment& a, const std::vector<external_function>& external_functions)
    {
    compile_expression(code, data, env, a.expr, external_functions);
    auto it = data.vars.find(a.name);
    if (it == data.vars.end())
      throw_compile_error(a.line_nr, "variable " + a.name + " is not declared.");
    if (it->second.st != constant)
      throw_compile_error(a.line_nr, "variable " + a.name + " cannot be assigned.");
    int64_t var_id = it->second.address;
    if (it->second.vt != real && it->second.vt != integer)
      throw_compile_error(a.line_nr, "I expect " + a.name + " to be an integer or a float.");
    if (it->second.vt == real && rt == RT_INTEGER)
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
            code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::MEM_RSP, real_offset);
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
            code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::MEM_RSP, int_offset);
            }
          else
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
            code.add(VM::vmcode::ADDSD, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::MEM_RSP, real_offset);
            }
          else
            {
            code.add(VM::vmcode::ADDSD, VM::vmcode::MEM_RSP, -var_id, real_op);
            }
          }
        else
          {
          if (int_offset)
            {
            code.add(VM::vmcode::ADD, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::MEM_RSP, int_offset);
            }
          else
            {
            code.add(VM::vmcode::ADD, VM::vmcode::MEM_RSP, -var_id, int_op);
            }
          }
        }
      else
        {
        if (rt == RT_REAL)
          {
          if (real_offset)
            {
            code.add(VM::vmcode::ADDSD, (VM::vmcode::operand)var_id, VM::vmcode::MEM_RSP, real_offset);
            }
          else
            {
            code.add(VM::vmcode::ADDSD, (VM::vmcode::operand)var_id, real_op);
            }
          }
        else
          {
          if (int_offset)
            {
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
            code.add(VM::vmcode::SUBSD, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::MEM_RSP, real_offset);
            }
          else
            {
            code.add(VM::vmcode::SUBSD, VM::vmcode::MEM_RSP, -var_id, real_op);
            }
          }
        else
          {
          if (int_offset)
            {
            code.add(VM::vmcode::SUB, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::MEM_RSP, int_offset);
            }
          else
            {
            code.add(VM::vmcode::SUB, VM::vmcode::MEM_RSP, -var_id, int_op);
            }
          }
        }
      else
        {
        if (rt == RT_REAL)
          {
          if (real_offset)
            {
            code.add(VM::vmcode::SUBSD, (VM::vmcode::operand)var_id, VM::vmcode::MEM_RSP, real_offset);
            }
          else
            {
            code.add(VM::vmcode::SUBSD, (VM::vmcode::operand)var_id, real_op);
            }
          }
        else
          {
          if (int_offset)
            {
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
            code.add(VM::vmcode::MULSD, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::MEM_RSP, real_offset);
            }
          else
            {
            code.add(VM::vmcode::MULSD, VM::vmcode::MEM_RSP, -var_id, real_op);
            }
          }
        else
          {
          if (int_offset)
            {
            code.add(VM::vmcode::IMUL, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::MEM_RSP, int_offset);
            }
          else
            {
            code.add(VM::vmcode::IMUL, VM::vmcode::MEM_RSP, -var_id, int_op);
            }
          }
        }
      else
        {
        if (rt == RT_REAL)
          {
          if (real_offset)
            {
            code.add(VM::vmcode::MULSD, (VM::vmcode::operand)var_id, VM::vmcode::MEM_RSP, real_offset);
            }
          else
            {
            code.add(VM::vmcode::MULSD, (VM::vmcode::operand)var_id, real_op);
            }
          }
        else
          {
          if (int_offset)
            {
            code.add(VM::vmcode::IMUL, (VM::vmcode::operand)var_id, VM::vmcode::MEM_RSP, int_offset);
            }
          else
            {
            code.add(VM::vmcode::IMUL, (VM::vmcode::operand)var_id, int_op);
            }
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
            code.add(VM::vmcode::DIVSD, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::MEM_RSP, real_offset);
            }
          else
            {
            code.add(VM::vmcode::DIVSD, VM::vmcode::MEM_RSP, -var_id, real_op);
            }
          }
        else
          {
          code.add(VM::vmcode::IDIV2, VM::vmcode::MEM_RSP, -var_id, int_op);
          }
        }
      else
        {
        if (rt == RT_REAL)
          {
          if (real_offset)
            {
            code.add(VM::vmcode::DIVSD, (VM::vmcode::operand)var_id, VM::vmcode::MEM_RSP, real_offset);
            }
          else
            {
            code.add(VM::vmcode::DIVSD, (VM::vmcode::operand)var_id, real_op);
            }
          }
        else
          {
          code.add(VM::vmcode::IDIV2, (VM::vmcode::operand)var_id, int_op);
          }
        }
      break;
      }
      default: throw std::runtime_error("not implemented");
      }
    }

  void compile_global_assignment_single(VM::vmcode& code, compile_data& data, environment& env, const Assignment& a, const std::vector<external_function>& external_functions)
    {
    compile_expression(code, data, env, a.expr, external_functions);
    auto it = env.globals.find(a.name);
    if (it == env.globals.end())
      throw_compile_error(a.line_nr, "global variable " + a.name + " is not declared.");
    int64_t address = it->second.address;
    if (it->second.vt != global_value_real && it->second.vt != global_value_integer)
      throw_compile_error(a.line_nr, "I expect " + a.name + " to be an integer or a float.");
    if (it->second.vt == global_value_real && rt == RT_INTEGER)
      {
      convert_integer_to_real(code, data.stack_index);
      rt = RT_REAL;
      }
    else if (it->second.vt == global_value_integer && rt == RT_REAL)
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
          code.add(VM::vmcode::MOV, GLOBAL_VARIABLE_MEM_REG, address, VM::vmcode::MEM_RSP, real_offset);
          }
        else
          {
          code.add(VM::vmcode::MOV, GLOBAL_VARIABLE_MEM_REG, address, real_op);
          }
        }
      else
        {
        if (int_offset)
          {
          code.add(VM::vmcode::MOV, GLOBAL_VARIABLE_MEM_REG, address, VM::vmcode::MEM_RSP, int_offset);
          }
        else
          code.add(VM::vmcode::MOV, GLOBAL_VARIABLE_MEM_REG, address, int_op);
        }
      break;
      }
      case '+':
      {

      if (rt == RT_REAL)
        {
        if (real_offset)
          {
          code.add(VM::vmcode::ADDSD, GLOBAL_VARIABLE_MEM_REG, address, VM::vmcode::MEM_RSP, real_offset);
          }
        else
          {
          code.add(VM::vmcode::ADDSD, GLOBAL_VARIABLE_MEM_REG, address, real_op);
          }
        }
      else
        {
        if (int_offset)
          {
          code.add(VM::vmcode::ADD, GLOBAL_VARIABLE_MEM_REG, address, VM::vmcode::MEM_RSP, int_offset);
          }
        else
          {
          code.add(VM::vmcode::ADD, GLOBAL_VARIABLE_MEM_REG, address, int_op);
          }
        }

      break;
      }
      case '-':
      {

      if (rt == RT_REAL)
        {
        if (real_offset)
          {
          code.add(VM::vmcode::SUBSD, GLOBAL_VARIABLE_MEM_REG, address, VM::vmcode::MEM_RSP, real_offset);
          }
        else
          {
          code.add(VM::vmcode::SUBSD, GLOBAL_VARIABLE_MEM_REG, address, real_op);
          }
        }
      else
        {
        if (int_offset)
          {
          code.add(VM::vmcode::SUB, GLOBAL_VARIABLE_MEM_REG, address, VM::vmcode::MEM_RSP, int_offset);
          }
        else
          {
          code.add(VM::vmcode::SUB, GLOBAL_VARIABLE_MEM_REG, address, int_op);
          }
        }
      break;
      }
      case '*':
      {
      if (rt == RT_REAL)
        {
        if (real_offset)
          {
          code.add(VM::vmcode::MULSD, GLOBAL_VARIABLE_MEM_REG, address, VM::vmcode::MEM_RSP, real_offset);
          }
        else
          {
          code.add(VM::vmcode::MULSD, GLOBAL_VARIABLE_MEM_REG, address, real_op);
          }
        }
      else
        {
        if (int_offset)
          {
          code.add(VM::vmcode::IMUL, GLOBAL_VARIABLE_MEM_REG, address, VM::vmcode::MEM_RSP, int_offset);
          }
        else
          {
          code.add(VM::vmcode::IMUL, GLOBAL_VARIABLE_MEM_REG, address, int_op);
          }
        }
      break;
      }
      case '/':
      {

      if (rt == RT_REAL)
        {
        if (real_offset)
          {
          code.add(VM::vmcode::DIVSD, GLOBAL_VARIABLE_MEM_REG, address, VM::vmcode::MEM_RSP, real_offset);
          }
        else
          {
          code.add(VM::vmcode::DIVSD, GLOBAL_VARIABLE_MEM_REG, address, real_op);
          }
        }
      else
        {
        code.add(VM::vmcode::IDIV2, GLOBAL_VARIABLE_MEM_REG, address, int_op);
        }
      break;
      }
      default: throw std::runtime_error("not implemented");
      }
    }

  void compile_assignment(VM::vmcode& code, compile_data& data, environment& env, const Assignment& a, const std::vector<external_function>& external_functions)
    {
    if (data.stack_index != 0)
      throw_compile_error(a.line_nr, "assignment only as single statement allowed");

    if (a.name.front() == '$')
      {
      auto it = env.globals.find(a.name);
      if (it == env.globals.end())
        throw_compile_error(a.line_nr, "global variable " + a.name + " is not declared.");
      if (!a.dims.empty())
        throw_compile_error(a.line_nr, "global variable " + a.name + " is not an array.");
      if (a.dereference)
        throw_compile_error(a.line_nr, "global variable " + a.name + " cannot be dereferenced.");
      compile_global_assignment_single(code, data, env, a, external_functions);
      }
    else
      {
      auto it = data.vars.find(a.name);
      if (it == data.vars.end())
        throw_compile_error(a.line_nr, "variable " + a.name + " is not declared.");

      if (!a.dims.empty())
        {
        if (it->second.vt == pointer_to_real || it->second.vt == pointer_to_integer)
          compile_assignment_pointer(code, data, env, a, external_functions);
        else
          compile_assignment_array(code, data, env, a, external_functions);
        }
      else if (a.dereference)
        compile_assignment_dereference(code, data, env, a, external_functions);
      else
        compile_assignment_single(code, data, env, a, external_functions);
      }
    }


  void compile_array_call(VM::vmcode& code, compile_data& data, environment& env, const ArrayCall& a, const std::vector<external_function>& external_functions)
    {
    auto it = data.vars.find(a.name);
    if (it == data.vars.end())
      throw_compile_error(a.line_nr, "variable " + a.name + " is unknown");
    if (a.exprs.size() != 1)
      throw_compile_error(a.line_nr, "only single dimension arrays are allowed.");
    if (it->second.vt == real || it->second.vt == integer)
      throw_compile_error(a.line_nr, "I expect an array or a pointer.");
    compile_expression(code, data, env, a.exprs.front(), external_functions);
    if (rt == RT_REAL)
      {
      convert_real_to_integer(code, data.stack_index);
      rt = RT_INTEGER;
      }
    int64_t var_id = it->second.address;

    if (it->second.vt == real_array)
      {
      if (it->second.at != memory_address)
        throw_compile_error(a.line_nr, "I expect " + a.name + " to be an array on the stack.");
      VM::vmcode::operand op;
      int64_t offset;
      index_to_integer_operand(op, offset, data.stack_index);
      if (offset)
        {
        code.add(VM::vmcode::SHL, VM::vmcode::MEM_RSP, offset, VM::vmcode::NUMBER, 3);
        }
      else
        {
        code.add(VM::vmcode::SHL, op, VM::vmcode::NUMBER, 3);
        }
      code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::RSP);
      code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, op);
      index_to_real_operand(op, offset, data.stack_index);
      if (offset)
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, STACK_MEM_BACKUP_REGISTER, -var_id);
      else
        code.add(VM::vmcode::MOV, op, STACK_MEM_BACKUP_REGISTER, -var_id);
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
        code.add(VM::vmcode::SHL, VM::vmcode::MEM_RSP, offset, VM::vmcode::NUMBER, 3);
        }
      else
        {
        code.add(VM::vmcode::SHL, op, VM::vmcode::NUMBER, 3);
        }
      code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::RSP);
      code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, op);
      if (offset)
        {
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, STACK_MEM_BACKUP_REGISTER, -var_id);
        }
      else
        code.add(VM::vmcode::MOV, op, STACK_MEM_BACKUP_REGISTER, -var_id);
      rt = RT_INTEGER;
      }
    else if (it->second.vt == pointer_to_real)
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
        code.add(VM::vmcode::SHL, VM::vmcode::MEM_RSP, offset, VM::vmcode::NUMBER, 3);
        code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, VM::vmcode::MEM_RSP, offset);
        }
      else
        {
        code.add(VM::vmcode::SHL, op, VM::vmcode::NUMBER, 3);
        code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, op);
        }
      index_to_real_operand(op, offset, data.stack_index);
      if (offset)
        {
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, STACK_MEM_BACKUP_REGISTER);
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
        code.add(VM::vmcode::SHL, VM::vmcode::MEM_RSP, offset, VM::vmcode::NUMBER, 3);
        code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, VM::vmcode::MEM_RSP, offset);
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, STACK_MEM_BACKUP_REGISTER);
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

  void compile_dereference(VM::vmcode& code, compile_data& data, environment& /*env*/, const Dereference& d)
    {
    auto it = data.vars.find(d.name);
    if (it == data.vars.end())
      throw_compile_error(d.line_nr, "variable " + d.name + " is unknown");
    if (it->second.vt != pointer_to_real && it->second.vt != pointer_to_integer)
      throw_compile_error(d.line_nr, "I expect a pointer to dereference.");
    int64_t var_id = it->second.address;
    if (it->second.at == memory_address)
      code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::MEM_RSP, -var_id);
    else
      code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, (VM::vmcode::operand)var_id);
    if (it->second.vt == pointer_to_real)
      {
      VM::vmcode::operand op;
      int64_t offset;
      index_to_real_operand(op, offset, data.stack_index);
      if (offset)
        {
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, STACK_MEM_BACKUP_REGISTER);
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
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, STACK_MEM_BACKUP_REGISTER);
        }
      else
        code.add(VM::vmcode::MOV, op, STACK_MEM_BACKUP_REGISTER);
      rt = RT_INTEGER;
      }
    }

  void compile_real(VM::vmcode& code, compile_data& data, environment& /*env*/, const value_t& v)
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
        code.add(VM::vmcode::MOV, op, VM::vmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&f)));
        }
      else
        {
        //code.add(VM::vmcode::XORPD, op, op);
        code.add(VM::vmcode::MOV, op, VM::vmcode::NUMBER, 0);
        }
      }
    rt = RT_REAL;
    }

  void compile_integer(VM::vmcode& code, compile_data& data, environment& /*env*/, const value_t& v)
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
      //if (i)
      code.add(VM::vmcode::MOV, op, VM::vmcode::NUMBER, i);
      //else
      //  code.add(VM::vmcode::XOR, op, op);
      }
    rt = RT_INTEGER;
    }

  void compile_value(VM::vmcode& code, compile_data& data, environment& env, const value_t& v)
    {
    if (std::holds_alternative<double>(v))
      compile_real(code, data, env, v);
    else
      compile_integer(code, data, env, v);
    }


  void compile_lvalue_operator(VM::vmcode& code, compile_data& data, environment& env, const LValueOperator& lvo, const std::vector<external_function>& external_functions)
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
      rt = (it->second.vt == real) ? RT_REAL : RT_INTEGER;
      if (rt == RT_INTEGER)
        {
        if (lvo.name == "++")
          {
          if (it->second.at == memory_address)
            code.add(VM::vmcode::ADD, VM::vmcode::MEM_RSP, -(int64_t)var_id, VM::vmcode::NUMBER, 1);
          else
            code.add(VM::vmcode::INC, (VM::vmcode::operand)var_id);
          }
        else if (lvo.name == "--")
          {
          if (it->second.at == memory_address)
            code.add(VM::vmcode::SUB, VM::vmcode::MEM_RSP, -(int64_t)var_id, VM::vmcode::NUMBER, 1);
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
            code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, VM::vmcode::MEM_RSP, -(int64_t)var_id);
            }
          else
            code.add(VM::vmcode::MOV, op, VM::vmcode::MEM_RSP, -(int64_t)var_id);
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
          double f = 1.0;
          if (lvo.name == "++")
            code.add(VM::vmcode::ADDSD, VM::vmcode::MEM_RSP, -(int64_t)var_id, VM::vmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&f)));
          else if (lvo.name == "--")
            code.add(VM::vmcode::SUBSD, VM::vmcode::MEM_RSP, -(int64_t)var_id, VM::vmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&f)));
          else
            throw_compile_error(lvo.line_nr, "operator " + lvo.name + " is unknown");
          VM::vmcode::operand op;
          int64_t offset;
          index_to_real_operand(op, offset, data.stack_index);
          if (offset == 0 && op == FIRST_TEMP_REAL_REG)  // send through the output to the output register
            {
            code.add(VM::vmcode::MOV, op, VM::vmcode::MEM_RSP, -(int64_t)var_id);
            }
          }
        else
          {
          double f = 1.0;
          if (lvo.name == "++")
            code.add(VM::vmcode::ADDSD, (VM::vmcode::operand)var_id, VM::vmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&f)));
          else if (lvo.name == "--")
            code.add(VM::vmcode::SUBSD, (VM::vmcode::operand)var_id, VM::vmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&f)));
          else
            throw_compile_error(lvo.line_nr, "operator " + lvo.name + " is unknown");
          VM::vmcode::operand op;
          int64_t offset;
          index_to_real_operand(op, offset, data.stack_index);
          if (offset == 0 && op == FIRST_TEMP_REAL_REG)
            code.add(VM::vmcode::MOV, FIRST_TEMP_REAL_REG, (VM::vmcode::operand)var_id); // send through the output to the output register
          }
        }
      }
    else if (std::holds_alternative<ArrayCall>(lvo.lvalue->lvalue))
      {
      ArrayCall a = std::get<ArrayCall>(lvo.lvalue->lvalue);
      auto it = data.vars.find(a.name);
      if (it == data.vars.end())
        throw_compile_error(a.line_nr, "Cannot find variable " + a.name);
      compile_expression(code, data, env, a.exprs.front(), external_functions);
      if (rt == RT_REAL)
        {
        convert_real_to_integer(code, data.stack_index);
        rt = RT_INTEGER;
        }
      int64_t var_id = it->second.address;

      if (it->second.vt == real_array)
        {
        if (it->second.at != memory_address)
          throw_compile_error(a.line_nr, "I expect " + a.name + " to be an array on the stack.");
        VM::vmcode::operand op;
        int64_t offset;
        index_to_integer_operand(op, offset, data.stack_index);
        if (offset)
          {
          code.add(VM::vmcode::SHL, VM::vmcode::MEM_RSP, offset, VM::vmcode::NUMBER, 3);
          code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::RSP);
          code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, VM::vmcode::MEM_RSP, offset);
          }
        else
          {
          code.add(VM::vmcode::SHL, op, VM::vmcode::NUMBER, 3);
          code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::RSP);
          code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, op);
          }
        double f = 1.0;
        if (lvo.name == "++")
          code.add(VM::vmcode::ADDSD, STACK_MEM_BACKUP_REGISTER, -var_id, VM::vmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&f)));
        else if (lvo.name == "--")
          code.add(VM::vmcode::SUBSD, STACK_MEM_BACKUP_REGISTER, -var_id, VM::vmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&f)));
        else
          throw_compile_error(lvo.line_nr, "operator " + lvo.name + " is unknown");
        index_to_real_operand(op, offset, data.stack_index);
        if (offset == 0 && op == FIRST_TEMP_REAL_REG)
          code.add(VM::vmcode::MOV, op, STACK_MEM_BACKUP_REGISTER, -var_id); // send through the output to the output register        

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
          code.add(VM::vmcode::SHL, VM::vmcode::MEM_RSP, offset, VM::vmcode::NUMBER, 3);
          code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::RSP);
          code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, VM::vmcode::MEM_RSP, offset);
          }
        else
          {
          code.add(VM::vmcode::SHL, op, VM::vmcode::NUMBER, 3);
          code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::RSP);
          code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, op);
          }
        if (lvo.name == "++")
          code.add(VM::vmcode::ADD, STACK_MEM_BACKUP_REGISTER, -var_id, VM::vmcode::NUMBER, 1);
        else if (lvo.name == "--")
          code.add(VM::vmcode::SUB, STACK_MEM_BACKUP_REGISTER, -var_id, VM::vmcode::NUMBER, 1);
        else
          throw_compile_error(lvo.line_nr, "operator " + lvo.name + " is unknown");
        if (offset)
          {
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, STACK_MEM_BACKUP_REGISTER, -var_id);
          }
        else
          code.add(VM::vmcode::MOV, op, STACK_MEM_BACKUP_REGISTER, -var_id);
        rt = RT_INTEGER;
        }
      else if (it->second.vt == pointer_to_real)
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
          code.add(VM::vmcode::SHL, VM::vmcode::MEM_RSP, offset, VM::vmcode::NUMBER, 3);
          code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, VM::vmcode::MEM_RSP, offset);
          }
        else
          {
          code.add(VM::vmcode::SHL, op, VM::vmcode::NUMBER, 3);
          code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, op);
          }
        index_to_real_operand(op, offset, data.stack_index);
        if (offset)
          {
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, STACK_MEM_BACKUP_REGISTER);
          }
        else
          code.add(VM::vmcode::MOV, op, STACK_MEM_BACKUP_REGISTER);
        double f = 1.0;
        if (lvo.name == "++")
          code.add(VM::vmcode::ADDSD, op, VM::vmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&f)));
        else if (lvo.name == "--")
          code.add(VM::vmcode::SUBSD, op, VM::vmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&f)));
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
          code.add(VM::vmcode::SHL, VM::vmcode::MEM_RSP, offset, VM::vmcode::NUMBER, 3);
          code.add(VM::vmcode::ADD, STACK_BACKUP_REGISTER, VM::vmcode::MEM_RSP, offset);
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
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, STACK_MEM_BACKUP_REGISTER);
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
      if (it->second.vt != pointer_to_real && it->second.vt != pointer_to_integer)
        throw_compile_error(d.line_nr, "I expect a pointer to dereference.");
      int64_t var_id = it->second.address;
      if (it->second.vt == pointer_to_real)
        {
        if (it->second.at == memory_address)
          code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, VM::vmcode::MEM_RSP, -var_id); // address to pointer
        else
          code.add(VM::vmcode::MOV, STACK_BACKUP_REGISTER, (VM::vmcode::operand)var_id); // address to pointer
        VM::vmcode::operand op;
        double f = 1.0;
        if (lvo.name == "++")
          code.add(VM::vmcode::ADDSD, STACK_MEM_BACKUP_REGISTER, VM::vmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&f)));
        else if (lvo.name == "--")
          code.add(VM::vmcode::SUBSD, STACK_MEM_BACKUP_REGISTER, VM::vmcode::NUMBER, *(reinterpret_cast<uint64_t*>(&f)));
        else
          throw_compile_error(lvo.line_nr, "operator " + lvo.name + " is unknown");
        int64_t offset;
        index_to_real_operand(op, offset, data.stack_index);
        if (offset == 0 && op == FIRST_TEMP_REAL_REG)
          code.add(VM::vmcode::MOV, op, STACK_MEM_BACKUP_REGISTER); // send through the output to the output register        
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
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, STACK_MEM_BACKUP_REGISTER);
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

  void compile_external_funccall(VM::vmcode& code, compile_data& data, environment& env, const FuncCall& f, const std::vector<external_function>& external_functions)
    {
    auto it = std::find_if(external_functions.begin(), external_functions.end(), [&](const auto& fun) { return f.name == fun.name; });
    if (it == external_functions.end())
      throw_compile_error(f.line_nr, "Invalid function");
    if (it->args.size() != f.exprs.size())
      throw_compile_error(f.line_nr, "Invalid number of arguments");
    if (it->args.size() > 4)
      throw_compile_error(f.line_nr, "Only max 4 arguments are possible for external functions");

    static std::vector<VM::vmcode::operand> external_regs = get_external_calling_registers();
    static std::vector<VM::vmcode::operand> external_float_regs = get_external_floating_point_registers();

    auto stack_save = data.stack_index;

    for (size_t i = 0; i < f.exprs.size(); ++i)
      {
      compile_expression(code, data, env, f.exprs[i], external_functions);
      if (it->args[i] == external_function_parameter_real)
        {
        if (rt == RT_INTEGER)
          {
          convert_integer_to_real(code, data.stack_index);
          rt = RT_REAL;
          }
        }
      ++data.stack_index;
      update_data(data);
      }

    int64_t return_type_save_stack_index = -1;

    switch (it->return_type)
      {
      case external_function_return_real:
      {
      return_type_save_stack_index = data.stack_index;
      VM::vmcode::operand op;
      int64_t offset;
      index_to_stack_operand(op, offset, data.stack_index);
      if (offset)
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, VM::vmcode::XMM0);
      else
        code.add(VM::vmcode::MOV, op, VM::vmcode::XMM0);
      ++data.stack_index;
      update_data(data);
      break;
      }
      case external_function_return_integer:
      {
      return_type_save_stack_index = data.stack_index;
      VM::vmcode::operand op;
      int64_t offset;
      index_to_stack_operand(op, offset, data.stack_index);
      if (offset)
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, VM::vmcode::RAX);
      else
        code.add(VM::vmcode::MOV, op, VM::vmcode::RAX);
      ++data.stack_index;
      update_data(data);
      break;
      }
      default:
        break;
      }

    std::vector<int64_t> argument_stack_indices;
    for (size_t i = 0; i < f.exprs.size(); ++i)
      {
      switch (it->args[i])
        {
        case external_function_parameter_real:
        {
        argument_stack_indices.push_back(data.stack_index);
        VM::vmcode::operand op;
        int64_t offset;
        index_to_stack_operand(op, offset, data.stack_index);
        if (offset)
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, external_float_regs[i]);
        else
          code.add(VM::vmcode::MOV, op, external_float_regs[i]);

        index_to_real_operand(op, offset, stack_save + i);
        if (offset)
          code.add(VM::vmcode::MOV, external_float_regs[i], VM::vmcode::MEM_RSP, offset);
        else
          code.add(VM::vmcode::MOV, external_float_regs[i], op);

        ++data.stack_index;
        update_data(data);
        break;
        }
        default:
        {
        argument_stack_indices.push_back(data.stack_index);
        VM::vmcode::operand op;
        int64_t offset;
        index_to_stack_operand(op, offset, data.stack_index);
        if (offset)
          code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, external_regs[i]);
        else
          code.add(VM::vmcode::MOV, op, external_regs[i]);

        index_to_integer_operand(op, offset, stack_save + i);
        if (offset)
          code.add(VM::vmcode::MOV, external_regs[i], VM::vmcode::MEM_RSP, offset);
        else
          code.add(VM::vmcode::MOV, external_regs[i], op);

        ++data.stack_index;
        update_data(data);
        break;
        }
        }
      }

    code.add(VM::vmcode::CALLEXTERNAL, VM::vmcode::NUMBER, (uint64_t)it->func_ptr);

    switch (it->return_type)
      {
      case external_function_return_real:
      {
      VM::vmcode::operand op;
      int64_t offset;
      index_to_real_operand(op, offset, stack_save);
      if (offset)
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, VM::vmcode::XMM0);
      else
        code.add(VM::vmcode::MOV, op, VM::vmcode::XMM0);

      index_to_stack_operand(op, offset, return_type_save_stack_index);
      if (offset)
        code.add(VM::vmcode::MOV, VM::vmcode::XMM0, VM::vmcode::MEM_RSP, offset);
      else
        code.add(VM::vmcode::MOV, VM::vmcode::XMM0, op);
      rt = RT_REAL;
      break;
      }
      case external_function_return_integer:
      {
      VM::vmcode::operand op;
      int64_t offset;
      index_to_integer_operand(op, offset, stack_save);
      if (offset)
        code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, offset, VM::vmcode::RAX);
      else
        code.add(VM::vmcode::MOV, op, VM::vmcode::RAX);

      index_to_stack_operand(op, offset, return_type_save_stack_index);
      if (offset)
        code.add(VM::vmcode::MOV, VM::vmcode::RAX, VM::vmcode::MEM_RSP, offset);
      else
        code.add(VM::vmcode::MOV, VM::vmcode::RAX, op);
      rt = RT_INTEGER;
      break;
      }
      default:
        break;
      }

    for (size_t i = 0; i < f.exprs.size(); ++i)
      {
      switch (it->args[i])
        {
        case external_function_parameter_real:
        {
        VM::vmcode::operand op;
        int64_t offset;
        index_to_stack_operand(op, offset, argument_stack_indices[i]);
        if (offset)
          code.add(VM::vmcode::MOV, external_float_regs[i], VM::vmcode::MEM_RSP, offset);
        else
          code.add(VM::vmcode::MOV, op, external_float_regs[i], op);
        break;
        }
        default:
        {
        VM::vmcode::operand op;
        int64_t offset;
        index_to_stack_operand(op, offset, argument_stack_indices[i]);
        if (offset)
          code.add(VM::vmcode::MOV, external_regs[i], VM::vmcode::MEM_RSP, offset);
        else
          code.add(VM::vmcode::MOV, external_regs[i], op);
        break;
        }
        }
      }

    data.stack_index = stack_save;
    }

  void compile_funccall(VM::vmcode& code, compile_data& data, environment& env, const FuncCall& f, const std::vector<external_function>& external_functions)
    {
    auto it = c_funcs.find(f.name);
    if (it == c_funcs.end())
      compile_external_funccall(code, data, env, f, external_functions);
    else
      {
      auto stack_save = data.stack_index;
      for (size_t i = 0; i < f.exprs.size(); ++i)
        {
        compile_expression(code, data, env, f.exprs[i], external_functions);
        if (rt == RT_INTEGER)
          {
          convert_integer_to_real(code, data.stack_index);
          rt = RT_REAL;
          }
        ++data.stack_index;
        update_data(data);
        }
      data.stack_index = stack_save;
      it->second(code, data);
      }
    }

  void compile_factor(VM::vmcode& code, compile_data& data, environment& env, const Factor& f, const std::vector<external_function>& external_functions)
    {
    if (std::holds_alternative<value_t>(f.factor))
      {
      compile_value(code, data, env, std::get<value_t>(f.factor));
      }
    else if (std::holds_alternative<Expression>(f.factor))
      {
      compile_expression(code, data, env, std::get<Expression>(f.factor), external_functions);
      }
    else if (std::holds_alternative<Variable>(f.factor))
      {
      compile_variable(code, data, env, std::get<Variable>(f.factor));
      }
    else if (std::holds_alternative<ArrayCall>(f.factor))
      {
      compile_array_call(code, data, env, std::get<ArrayCall>(f.factor), external_functions);
      }
    else if (std::holds_alternative<Dereference>(f.factor))
      {
      compile_dereference(code, data, env, std::get<Dereference>(f.factor));
      }
    else if (std::holds_alternative<LValueOperator>(f.factor))
      {
      compile_lvalue_operator(code, data, env, std::get<LValueOperator>(f.factor), external_functions);
      }
    else if (std::holds_alternative<FuncCall>(f.factor))
      {
      compile_funccall(code, data, env, std::get<FuncCall>(f.factor), external_functions);
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

  void compile_term(VM::vmcode& code, compile_data& data, environment& env, const Term& t, const std::vector<external_function>& external_functions)
    {
    compile_factor(code, data, env, t.operands[0], external_functions);
    auto target_type = rt;
    for (size_t i = 0; i < t.fops.size(); ++i)
      {
      ++data.stack_index;
      update_data(data);
      compile_factor(code, data, env, t.operands[i + 1], external_functions);
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

  void compile_relop(VM::vmcode& code, compile_data& data, environment& env, const Relop& r, const std::vector<external_function>& external_functions)
    {
    compile_term(code, data, env, r.operands[0], external_functions);
    auto target_type = rt;
    for (size_t i = 0; i < r.fops.size(); ++i)
      {
      ++data.stack_index;
      update_data(data);
      compile_term(code, data, env, r.operands[i + 1], external_functions);
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

  void compile_expression(VM::vmcode& code, compile_data& data, environment& env, const Expression& expr, const std::vector<external_function>& external_functions)
    {
    compile_relop(code, data, env, expr.operands[0], external_functions);
    auto target_type = rt;
    for (size_t i = 0; i < expr.fops.size(); ++i)
      {
      ++data.stack_index;
      update_data(data);
      compile_relop(code, data, env, expr.operands[i + 1], external_functions);
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

  void compile_for_condition(VM::vmcode& code, compile_data& data, environment& env, const Statement& stm, uint64_t end_label, int line_nr, const std::vector<external_function>& external_functions)
    {
    if (is_simple_relative_operation(stm))
      {
      if (data.stack_index != 0)
        throw_compile_error(line_nr, "for loop not expected as an expression");
      const Expression& expr = std::get<Expression>(stm);
      assert(expr.fops.size() == 1);
      assert(expr.operands.size() == 2);
      compile_relop(code, data, env, expr.operands[0], external_functions);
      auto target_type = rt;
      ++data.stack_index;
      update_data(data);
      compile_relop(code, data, env, expr.operands[1], external_functions);
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
        code.add(VM::vmcode::CMP, FIRST_TEMP_INTEGER_REG, VM::vmcode::NUMBER, 0);
        code.add(VM::vmcode::JE, label_to_string(end_label));
        }
      else
        {
        --data.stack_index;
        code.add(VM::vmcode::CMP, FIRST_TEMP_INTEGER_REG, SECOND_TEMP_INTEGER_REG);
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
      compile_statement(code, data, env, stm, external_functions);
      if (rt != RT_INTEGER)
        throw_compile_error(line_nr, "for loop condition is not a boolean expression");
      if (data.stack_index != 0)
        throw_compile_error(line_nr, "for loop not expected as an expression");
      code.add(VM::vmcode::CMP, FIRST_TEMP_INTEGER_REG, VM::vmcode::NUMBER, 0);
      code.add(VM::vmcode::JE, label_to_string(end_label));
      }
    }

  void compile_for(VM::vmcode& code, compile_data& data, environment& env, const For& f, const std::vector<external_function>& external_functions)
    {
    compile_statement(code, data, env, f.init_cond_inc[0], external_functions);
    auto start = data.label++;
    auto end = data.label++;
    code.add(VM::vmcode::LABEL, label_to_string(start));
    compile_for_condition(code, data, env, f.init_cond_inc[1], end, f.line_nr, external_functions);
    for (const auto& stm : f.statements)
      compile_statement(code, data, env, stm, external_functions);
    compile_statement(code, data, env, f.init_cond_inc[2], external_functions);
    auto last_instruction = code.get_instructions_list().back().back();
    while (last_instruction.oper == VM::vmcode::MOV && last_instruction.operand1_mem == 0 &&
      (last_instruction.operand1 == FIRST_TEMP_INTEGER_REG || last_instruction.operand1 == SECOND_TEMP_INTEGER_REG ||
        last_instruction.operand1 == FIRST_TEMP_REAL_REG || last_instruction.operand1 == SECOND_TEMP_REAL_REG))
      {
      // This is an optimalization.
      // The increment condition does not need to be passed to one of the temp registers.
      code.get_instructions_list().back().pop_back();
      last_instruction = code.get_instructions_list().back().back();
      }
    code.add(VM::vmcode::JMP, label_to_string(start));
    code.add(VM::vmcode::LABEL, label_to_string(end));
    }

  void compile_if(VM::vmcode& code, compile_data& data, environment& env, const If& i, const std::vector<external_function>& external_functions)
    {
    compile_statement(code, data, env, i.condition[0], external_functions);
    if (rt != RT_INTEGER)
      throw_compile_error(i.line_nr, "if condition is not a boolean expression");
    if (data.stack_index != 0)
      throw_compile_error(i.line_nr, "if statement not expected as an expression");
    code.add(VM::vmcode::CMP, FIRST_TEMP_INTEGER_REG, VM::vmcode::NUMBER, 0);
    auto end_label = data.label++;
    if (i.alternative.empty())
      {
      code.add(VM::vmcode::JE, label_to_string(end_label));
      for (const auto& stm : i.body)
        compile_statement(code, data, env, stm, external_functions);
      }
    else
      {
      auto else_label = data.label++;
      code.add(VM::vmcode::JE, label_to_string(else_label));
      for (const auto& stm : i.body)
        compile_statement(code, data, env, stm, external_functions);
      code.add(VM::vmcode::JMP, label_to_string(end_label));
      code.add(VM::vmcode::LABEL, label_to_string(else_label));
      for (const auto& stm : i.alternative)
        compile_statement(code, data, env, stm, external_functions);
      }
    code.add(VM::vmcode::LABEL, label_to_string(end_label));
    }

  void compile_seperated_statements(VM::vmcode& code, compile_data& data, environment& env, const CommaSeparatedStatements& stms, const std::vector<external_function>& external_functions)
    {
    for (const auto& stm : stms.statements)
      compile_statement(code, data, env, stm, external_functions);
    }

  void compile_statement(VM::vmcode& code, compile_data& data, environment& env, const Statement& stm, const std::vector<external_function>& external_functions)
    {
    if (std::holds_alternative<Expression>(stm))
      {
      compile_expression(code, data, env, std::get<Expression>(stm), external_functions);
      }
    else if (std::holds_alternative<Float>(stm))
      {
      compile_make_float(code, data, env, std::get<Float>(stm), external_functions);
      }
    else if (std::holds_alternative<Int>(stm))
      {
      compile_make_int(code, data, env, std::get<Int>(stm), external_functions);
      }
    else if (std::holds_alternative<Assignment>(stm))
      {
      compile_assignment(code, data, env, std::get<Assignment>(stm), external_functions);
      }
    else if (std::holds_alternative<For>(stm))
      {
      compile_for(code, data, env, std::get<For>(stm), external_functions);
      }
    else if (std::holds_alternative<If>(stm))
      {
      compile_if(code, data, env, std::get<If>(stm), external_functions);
      }
    else if (std::holds_alternative<CommaSeparatedStatements>(stm))
      {
      compile_seperated_statements(code, data, env, std::get<CommaSeparatedStatements>(stm), external_functions);
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

  void compile_int_parameter_pointer(VM::vmcode& code, compile_data& data, environment& /*env*/, const IntParameter& ip, int32_t parameter_id, int32_t nr_of_pars)
    {
    auto it = data.vars.find(ip.name);
    if (it != data.vars.end())
      throw_compile_error(ip.line_nr, "Variable " + ip.name + " already exists");

    if (parameter_id < 4)
      {
      VM::vmcode::operand op = CALLING_CONVENTION_INT_PAR_1;
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
      //int32_t addr = parameter_id - 4;
      int32_t addr = nr_of_pars - parameter_id - 1;
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::MEM_RSP, rsp_offset + addr * 8 + virtual_machine_rsp_offset);
      data.vars.insert(std::make_pair(ip.name, make_variable(var_id, external, pointer_to_integer, memory_address)));
      }
    }

  void compile_int_parameter_single(VM::vmcode& code, compile_data& data, environment& /*env*/, const IntParameter& ip, int32_t parameter_id, int32_t nr_of_pars)
    {
    auto it = data.vars.find(ip.name);
    if (it != data.vars.end())
      throw_compile_error(ip.line_nr, "Variable " + ip.name + " already exists");

    if (parameter_id < 4)
      {
      VM::vmcode::operand op = CALLING_CONVENTION_INT_PAR_1;
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
      //int32_t addr = parameter_id - 4;
      int32_t addr = nr_of_pars - parameter_id - 1;
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::MEM_RSP, rsp_offset + addr * 8 + virtual_machine_rsp_offset);
      data.vars.insert(std::make_pair(ip.name, make_variable(var_id, constant, integer, memory_address)));
      }
    }

  void compile_int_parameter(VM::vmcode& code, compile_data& data, environment& env, const IntParameter& ip, int parameter_id, int32_t nr_of_pars)
    {
    if (ip.pointer)
      compile_int_parameter_pointer(code, data, env, ip, parameter_id, nr_of_pars);
    else
      compile_int_parameter_single(code, data, env, ip, parameter_id, nr_of_pars);
    }


  void compile_float_parameter_pointer(VM::vmcode& code, compile_data& data, environment& /*env*/, const FloatParameter& fp, int parameter_id, int32_t nr_of_pars)
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
      //int32_t addr = parameter_id - 4;
      int32_t addr = nr_of_pars - parameter_id - 1;
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::MEM_RSP, rsp_offset + addr * 8 + virtual_machine_rsp_offset);
      }
    auto it = data.vars.find(fp.name);
    if (it == data.vars.end())
      data.vars.insert(std::make_pair(fp.name, make_variable(var_id, external, pointer_to_real, memory_address)));
    else
      throw_compile_error(fp.line_nr, "Variable " + fp.name + " already exists");
    }

  void compile_float_parameter_single(VM::vmcode& code, compile_data& data, environment& /*env*/, const FloatParameter& fp, int32_t parameter_id, int32_t nr_of_pars)
    {
    auto it = data.vars.find(fp.name);
    if (it != data.vars.end())
      throw_compile_error(fp.line_nr, "Variable " + fp.name + " already exists");

    if (parameter_id < 4)
      {
      VM::vmcode::operand op = CALLING_CONVENTION_REAL_PAR_1;
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
      data.vars.insert(std::make_pair(fp.name, make_variable(op, constant, real, register_address)));
      data.ra.make_real_register_unavailable(op);
      }
    else
      {
      int64_t var_id = data.var_offset | variable_tag;
      data.var_offset += 8;
      //int32_t addr = parameter_id - 4;
      int32_t addr = nr_of_pars - parameter_id - 1;
      code.add(VM::vmcode::MOV, VM::vmcode::MEM_RSP, -var_id, VM::vmcode::MEM_RSP, rsp_offset + addr * 8 + virtual_machine_rsp_offset);
      data.vars.insert(std::make_pair(fp.name, make_variable(var_id, constant, real, memory_address)));
      }
    }

  void compile_float_parameter(VM::vmcode& code, compile_data& data, environment& env, const FloatParameter& fp, int32_t parameter_id, int32_t nr_of_pars)
    {
    if (fp.pointer)
      compile_float_parameter_pointer(code, data, env, fp, parameter_id, nr_of_pars);
    else
      compile_float_parameter_single(code, data, env, fp, parameter_id, nr_of_pars);
    }

  void compile_parameters(VM::vmcode& code, compile_data& data, environment& env, const Parameters& pars)
    {
    int calling_convention_id = 0;
    for (auto par : pars)
      {
      if (std::holds_alternative<IntParameter>(par))
        {
        compile_int_parameter(code, data, env, std::get<IntParameter>(par), calling_convention_id, (int32_t)pars.size());
        ++calling_convention_id;
        }
      else if (std::holds_alternative<FloatParameter>(par))
        {
        compile_float_parameter(code, data, env, std::get<FloatParameter>(par), calling_convention_id, (int32_t)pars.size());
        ++calling_convention_id;
        }
      else
        throw_compile_error("parameter type not implemented");
      }
    }

  } // anonymous namespace

void compile(VM::vmcode& code, environment& env, const Program& prog, const std::vector<external_function>& external_functions)
  {
  compile_data data;

  if constexpr (GLOBAL_VARIABLE_REG != VM::vmcode::RBP)
    code.add(VM::vmcode::MOV, GLOBAL_VARIABLE_REG, VM::vmcode::RBP); // pointer to global variables

  if constexpr (rsp_offset)
    code.add(VM::vmcode::SUB, VM::vmcode::RSP, VM::vmcode::NUMBER, rsp_offset);
  compile_parameters(code, data, env, prog.parameters);

  for (const auto& stm : prog.statements)
    compile_statement(code, data, env, stm, external_functions);
  if (rt == RT_INTEGER)
    {
    code.add(VM::vmcode::CVTSI2SD, VM::vmcode::XMM0, FIRST_TEMP_INTEGER_REG);
    }
  else if constexpr (FIRST_TEMP_REAL_REG != VM::vmcode::XMM0)
    {
    code.add(VM::vmcode::MOV, VM::vmcode::XMM0, FIRST_TEMP_REAL_REG);
    }
  if constexpr (rsp_offset)
    code.add(VM::vmcode::ADD, VM::vmcode::RSP, VM::vmcode::NUMBER, rsp_offset);
  code.add(VM::vmcode::RET);

  offset_stack(code, data);
  }

void compile(VM::vmcode& code, environment& env, const Program& prog)
  {
  std::vector<external_function> external_functions;
  compile(code, env, prog, external_functions);
  }

environment::environment() : global_var_offset(0)
  {
  }

COMPILER_END
