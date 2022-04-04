#pragma once

#include "namespace.h"
#include "vm/vmcode.h"

#include "cscript_api.h"
#include "parse.h"

#include <stdint.h>
#include <string>
#include <map>

COMPILER_BEGIN

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

struct variable_type
  {
  int64_t address;
  storage_type st;
  value_type vt;
  };

inline variable_type make_variable(int64_t addr, storage_type st, value_type vt)
  {
  variable_type ret;
  ret.address = addr;
  ret.st = st;
  ret.vt = vt;
  return ret;
  }

typedef std::map<std::string, variable_type> variable_map;

struct compile_data
  {
  int64_t max_stack_index;
  int64_t stack_index;
  variable_map vars;
  int64_t var_offset;
  uint64_t label;
  };

COMPILER_CSCRIPT_API void compile(VM::vmcode& code, compile_data& data, const Program& prog);

COMPILER_END