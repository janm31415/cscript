#pragma once

#include "namespace.h"
#include "compiler.h"
#include "vm/vm.h"

#include "cscript_api.h"
#include <string>
#include <memory>

COMPILER_BEGIN

struct cscript_environment
  {
  environment env;
  VM::registers reg;
  };

enum class cscript_argument_type
  {
  cpp_int64,
  cpp_double,
  cpp_pointer_to_int64,
  cpp_pointer_to_double
  };

union cscript_argument_value
  {
  uint64_t i;
  double d;
  };

struct cscript_argument
  {
  cscript_argument_type type;
  cscript_argument_value value;
  };

COMPILER_CSCRIPT_API cscript_argument make_cscript_argument(int64_t i);
COMPILER_CSCRIPT_API cscript_argument make_cscript_argument(uint64_t i);
COMPILER_CSCRIPT_API cscript_argument make_cscript_argument(double d);
COMPILER_CSCRIPT_API cscript_argument make_cscript_argument(int64_t* p_i);
COMPILER_CSCRIPT_API cscript_argument make_cscript_argument(uint64_t* p_i);
COMPILER_CSCRIPT_API cscript_argument make_cscript_argument(double* p_d);

COMPILER_CSCRIPT_API bool make_cscript_global_variable(const std::string& variable_name, double variable_value, cscript_environment& env);
COMPILER_CSCRIPT_API bool make_cscript_global_variable(const std::string& variable_name, int64_t variable_value, cscript_environment& env);

class cscript_function
  {
  public:
    COMPILER_CSCRIPT_API ~cscript_function();

    COMPILER_CSCRIPT_API static std::unique_ptr<cscript_function> create(const std::string& script, cscript_environment& env, std::string& error_message);

    COMPILER_CSCRIPT_API double run(const std::vector<cscript_argument>& args, cscript_environment& env);

  private:
    cscript_function(uint8_t* bytecode, uint64_t bytecode_size);

  private:
    uint8_t* _bytecode;
    uint64_t _bytecode_size;
    std::string _last_error;
  };

COMPILER_END