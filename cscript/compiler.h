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

enum external_function_parameter_type
  {
  external_function_parameter_real,
  external_function_parameter_integer,
  external_function_parameter_pointer_to_real,
  external_function_parameter_pointer_to_integer
  };

enum external_function_return_type
  {
  external_function_return_real,
  external_function_return_integer,
  external_function_return_void
  };

struct external_function
  {
  std::string name;
  void* func_ptr;
  std::vector<external_function_parameter_type> args;
  external_function_return_type return_type;
  };

COMPILER_CSCRIPT_API void compile(VM::vmcode& code, environment& env, const Program& prog, const std::vector<external_function>& external_functions);
COMPILER_CSCRIPT_API void compile(VM::vmcode& code, environment& env, const Program& prog);

COMPILER_END