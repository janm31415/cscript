#pragma once

#include "namespace.h"
#include "vm/vmcode.h"

#include <map>
#include <set>
#include <vector>

COMPILER_BEGIN

class regalloc
  {
  public:
    regalloc(const std::vector<VM::vmcode::operand>& usable_integer_registers, const std::vector<VM::vmcode::operand>& usable_real_registers);
    ~regalloc();

    bool free_integer_register_available() const;
    VM::vmcode::operand get_next_available_integer_register();
    void make_integer_register_available(VM::vmcode::operand reg);
    uint8_t number_of_integer_registers();
    uint8_t number_of_available_integer_registers();
    bool is_free_integer_register(VM::vmcode::operand reg);
    void make_integer_register_unavailable(VM::vmcode::operand reg);

    bool free_real_register_available() const;
    VM::vmcode::operand get_next_available_real_register();
    void make_real_register_available(VM::vmcode::operand reg);
    uint8_t number_of_real_registers();
    uint8_t number_of_available_real_registers();
    bool is_free_real_register(VM::vmcode::operand reg);
    void make_real_register_unavailable(VM::vmcode::operand reg);

    void make_all_available();
    

    const std::vector<VM::vmcode::operand>& integer_registers() const;
    const std::vector<VM::vmcode::operand>& real_registers() const;

  private:
    std::vector<VM::vmcode::operand> available_integer_registers;
    std::map<VM::vmcode::operand, uint8_t> integer_register_to_index;
    std::set<uint8_t> free_integer_registers;

    std::vector<VM::vmcode::operand> available_real_registers;
    std::map<VM::vmcode::operand, uint8_t> real_register_to_index;
    std::set<uint8_t> free_real_registers;
  };

COMPILER_END