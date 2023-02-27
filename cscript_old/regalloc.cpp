#include "regalloc.h"
#include <cassert>

COMPILER_BEGIN

regalloc::regalloc(const std::vector<VM::vmcode::operand>& usable_integer_registers, const std::vector<VM::vmcode::operand>& usable_real_registers) 
  : available_integer_registers(usable_integer_registers), available_real_registers(usable_real_registers)
  {
  make_all_available();
  }

regalloc::~regalloc()
  {
  }

bool regalloc::free_integer_register_available() const
  {
  return !free_integer_registers.empty();
  }

VM::vmcode::operand regalloc::get_next_available_integer_register()
  {
  if (!free_integer_register_available())
    throw std::runtime_error("no integer_register available");
  assert(!free_integer_registers.empty());
  auto it = (--free_integer_registers.end());
  uint8_t reg_value = *it;
  free_integer_registers.erase(it);
  return available_integer_registers[reg_value];
  }

void regalloc::make_integer_register_available(VM::vmcode::operand reg)
  {
  assert(free_integer_registers.find(integer_register_to_index[reg]) == free_integer_registers.end());
  assert(integer_register_to_index.find(reg) != integer_register_to_index.end());
  free_integer_registers.insert(integer_register_to_index.at(reg));
  }

uint8_t regalloc::number_of_integer_registers()
  {
  return (uint8_t)available_integer_registers.size();
  }

uint8_t regalloc::number_of_available_integer_registers()
  {
  return (uint8_t)free_integer_registers.size();
  }

bool regalloc::is_free_integer_register(VM::vmcode::operand reg)
  {
  auto it = integer_register_to_index.find(reg);
  if (it == integer_register_to_index.end())
    return true;
  return (free_integer_registers.find(it->second) != free_integer_registers.end());
  }

void regalloc::make_all_available()
  {
  for (uint8_t i = 0; i < (uint8_t)available_integer_registers.size(); ++i)
    {
    integer_register_to_index[available_integer_registers[i]] = i;    
    free_integer_registers.insert(i);
    }
  for (uint8_t i = 0; i < (uint8_t)available_real_registers.size(); ++i)
    {
    real_register_to_index[available_real_registers[i]] = i;
    free_real_registers.insert(i);
    }
  }

void regalloc::make_integer_register_unavailable(VM::vmcode::operand reg)
  {
  free_integer_registers.erase(integer_register_to_index[reg]);
  }

const std::vector<VM::vmcode::operand>& regalloc::integer_registers() const
  { 
  return available_integer_registers; 
  }

bool regalloc::free_real_register_available() const
  {
  return !free_real_registers.empty();
  }

VM::vmcode::operand regalloc::get_next_available_real_register()
  {
  if (!free_real_register_available())
    throw std::runtime_error("no real_register available");
  assert(!free_real_registers.empty());
  auto it = (--free_real_registers.end());
  uint8_t reg_value = *it;
  free_real_registers.erase(it);
  return available_real_registers[reg_value];
  }

void regalloc::make_real_register_available(VM::vmcode::operand reg)
  {
  assert(free_real_registers.find(real_register_to_index[reg]) == free_real_registers.end());
  assert(real_register_to_index.find(reg) != real_register_to_index.end());
  free_real_registers.insert(real_register_to_index.at(reg));
  }

uint8_t regalloc::number_of_real_registers()
  {
  return (uint8_t)available_real_registers.size();
  }

uint8_t regalloc::number_of_available_real_registers()
  {
  return (uint8_t)free_real_registers.size();
  }

bool regalloc::is_free_real_register(VM::vmcode::operand reg)
  {
  auto it = real_register_to_index.find(reg);
  if (it == real_register_to_index.end())
    return true;
  return (free_real_registers.find(it->second) != free_real_registers.end());
  }

void regalloc::make_real_register_unavailable(VM::vmcode::operand reg)
  {
  free_real_registers.erase(real_register_to_index[reg]);
  }

const std::vector<VM::vmcode::operand>& regalloc::real_registers() const
  {
  return available_real_registers;
  }

COMPILER_END