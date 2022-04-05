#include "peephole.h"
#include "defines.h"

COMPILER_BEGIN

namespace
  {
  bool operand_is_temp(VM::vmcode::operand op)
    {
    return op == FIRST_TEMP_INTEGER_REG || op == SECOND_TEMP_INTEGER_REG || op == FIRST_TEMP_REAL_REG || op == SECOND_TEMP_REAL_REG;
    }

  bool operation_does_not_modify_first_operand(VM::vmcode::operation op)
    {
    switch (op)
      {
      case VM::vmcode::CMP: 
      case VM::vmcode::TEST:
        return true;
      default: return false;
      }
    }

  std::vector<VM::vmcode::instruction>::iterator peephole_mov(std::vector<VM::vmcode::instruction>::iterator it, std::vector<VM::vmcode::instruction>& vec)
    {
    if (it->oper != VM::vmcode::MOV)
      return it;
    if (it->operand1_mem != 0)
      return it;
    if (!operand_is_temp(it->operand1))
      return it;
    auto it2 = it;
    ++it2;
    if (it2 == vec.end())
      return it;
    if (it2->operand2_mem == 0 && it2->operand2 == it->operand1)
      {
      // we have the case
      //   mov tmp_reg, expr
      //   op  target_reg, tmp_reg
      // which can be replaced by
      //   op  target_reg, expr
      it2->operand2 = it->operand2;
      it2->operand2_mem = it->operand2_mem;
      it = vec.erase(it);
      }
    else if (it2->operand1_mem == 0 && it2->operand1 == it->operand1)
      {
      // we have the case
      //   mov tmp_reg, expr
      //   op  tmp_reg, other_reg
      // which can be replaced by
      //   op  expr, other_reg
      // if op is an operation that does not change expr.
      if (operation_does_not_modify_first_operand(it2->oper))
        {
        it2->operand1 = it->operand2;
        it2->operand1_mem = it->operand2_mem;
        it = vec.erase(it);
        }
      }
    return it;
    }

  void peephole(std::vector<VM::vmcode::instruction>& vec)
    {
    auto it = vec.begin();
    while (it != vec.end())
      {
      it = peephole_mov(it, vec);

      ++it;
      }
    }
  }

void peephole_optimization(VM::vmcode& code)
  {
  for (auto& list : code.get_instructions_list())
    {
    peephole(list);
    peephole(list);
    peephole(list);
    }
  }

COMPILER_END