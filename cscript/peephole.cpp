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

  bool operation_on_temp_register_can_be_done_on_target_register(VM::vmcode::instruction instr)
    {
    switch (instr.oper)
      {
      case VM::vmcode::XOR:
      case VM::vmcode::XORPD:
        return instr.operand1 == instr.operand2 && instr.operand1_mem == instr.operand2_mem;
      case VM::vmcode::MOV:
      case VM::vmcode::CVTSI2SD:
      case VM::vmcode::CVTTSD2SI:
      case VM::vmcode::SETE:
      case VM::vmcode::SETNE:
      case VM::vmcode::SETL:
      case VM::vmcode::SETG:
      case VM::vmcode::SETLE:
      case VM::vmcode::SETGE:
        return true;
      default:
        return false;
      }
    return false;
    }

  std::vector<VM::vmcode::instruction>::iterator peephole_mov_1(std::vector<VM::vmcode::instruction>::iterator it, std::vector<VM::vmcode::instruction>& vec)
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

  std::vector<VM::vmcode::instruction>::iterator peephole_mov_2(std::vector<VM::vmcode::instruction>::iterator it, std::vector<VM::vmcode::instruction>& vec)
    {
    if (it->oper != VM::vmcode::MOV)
      return it;
    if (it->operand2_mem != 0)
      return it;
    if (!operand_is_temp(it->operand2))
      return it;
    if (it == vec.begin())
      return it;
    auto it2 = it;
    --it2;
    if (it2->operand1_mem == 0 && it2->operand1 == it->operand2 && operation_on_temp_register_can_be_done_on_target_register(*it2))
      {
      // we have the case
      //   op  tmp_reg, expr
      //   mov other_reg, tmp_reg
      // which can be replaced by
      //   op  other_reg, expr
      // if op is an operation that does not use the value in tmp_reg.
      it2->operand1_mem = it->operand1_mem;
      it2->operand1 = it->operand1;
      if (it2->operand2_mem == 0 && it2->operand2 == it->operand2)
        {
        it2->operand2_mem = it->operand1_mem;
        it2->operand2 = it->operand1;
        }
      it = vec.erase(it);
      }
    return it;
    }

  std::vector<VM::vmcode::instruction>::iterator peephole_mov_3(std::vector<VM::vmcode::instruction>::iterator it, std::vector<VM::vmcode::instruction>& vec)
    {
    if (it->oper != VM::vmcode::MOV)
      return it;
    auto it2 = it;
    ++it2;
    if (it2 == vec.end())
      return it;
    if (it2->oper == VM::vmcode::MOV && it->operand1 == it2->operand1 && it->operand1_mem == it2->operand1_mem)
      {
      it = vec.erase(it); // two moves that move to the same spot, so remove the first move.
      }
    return it;
    }

  bool operations_duplicate_has_no_effect(VM::vmcode::instruction instr)
    {
    switch (instr.oper)
      {
      case VM::vmcode::AND:
      case VM::vmcode::OR:
      case VM::vmcode::CMP:
      case VM::vmcode::CMPEQPD:
      case VM::vmcode::CMPLTPD:
      case VM::vmcode::CMPLEPD:
      case VM::vmcode::TEST:
      case VM::vmcode::MOV:
      case VM::vmcode::SETE:
      case VM::vmcode::SETNE:
      case VM::vmcode::SETL:
      case VM::vmcode::SETG:
      case VM::vmcode::SETLE:
      case VM::vmcode::SETGE:
      case VM::vmcode::CVTSI2SD:
      case VM::vmcode::CVTTSD2SI:
      case VM::vmcode::CABS:
      case VM::vmcode::JMP:
      case VM::vmcode::JA:
      case VM::vmcode::JB:
      case VM::vmcode::JE:
      case VM::vmcode::JL:
      case VM::vmcode::JLE:
      case VM::vmcode::JG:
      case VM::vmcode::JGE:
      case VM::vmcode::JNE:
      case VM::vmcode::JMPS:
      case VM::vmcode::JAS:
      case VM::vmcode::JBS:
      case VM::vmcode::JES:
      case VM::vmcode::JLS:
      case VM::vmcode::JLES:
      case VM::vmcode::JGS:
      case VM::vmcode::JGES:
      case VM::vmcode::JNES:
        return true;
      default:
        return false;
      }
    return false;
    }

  std::vector<VM::vmcode::instruction>::iterator remove_duplicates(std::vector<VM::vmcode::instruction>::iterator it, std::vector<VM::vmcode::instruction>& vec)
    {
    if (!operations_duplicate_has_no_effect(it->oper))
      return it;
    auto it2 = it;
    ++it2;
    if (it2 == vec.end())
      return it;
    if (it2->oper == it->oper && it2->operand1 == it->operand1 && it2->operand1_mem == it->operand1_mem &&
      it2->operand2 == it->operand2 && it2->operand2_mem == it->operand2_mem)
      {
      it = vec.erase(it);
      }
    return it;
    }

  void peephole(std::vector<VM::vmcode::instruction>& vec)
    {
    auto it = vec.begin();
    while (it != vec.end())
      {
      it = peephole_mov_1(it, vec);
      it = peephole_mov_2(it, vec);
      it = peephole_mov_3(it, vec);
      it = remove_duplicates(it, vec);
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