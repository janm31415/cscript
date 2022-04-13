#pragma once

#include "namespace.h"
#include "vm/vmcode.h"

#include "cscript_api.h"
#include "parse.h"

#include <map>

COMPILER_BEGIN

enum global_value_type
  {
  global_value_real,
  global_value_integer,
  //global_value_real_array,
  //global_value_integer_array
  };

struct global_variable_type
  {
  int64_t address;
  global_value_type vt;
  };

typedef std::map<std::string, global_variable_type> global_variables_map;

struct environment
  {
  COMPILER_CSCRIPT_API environment();
  global_variables_map globals;
  int64_t global_var_offset;
  };

COMPILER_CSCRIPT_API void compile(VM::vmcode& code, environment& env, const Program& prog);

COMPILER_END