#include "cscript.h"
#include "compiler.h"
#include "optimize.h"
#include "parse.h"
#include "tokenize.h"
#include "peephole.h"

COMPILER_BEGIN

cscript_argument make_cscript_argument(int64_t i)
  {
  cscript_argument arg;
  arg.type = cscript_argument_type::cpp_int64;
  arg.value.i = (uint64_t)i;
  return arg;
  }

cscript_argument make_cscript_argument(uint64_t i)
  {
  cscript_argument arg;
  arg.type = cscript_argument_type::cpp_int64;
  arg.value.i = i;
  return arg;
  }

cscript_argument make_cscript_argument(double d)
  {
  cscript_argument arg;
  arg.type = cscript_argument_type::cpp_double;
  arg.value.d = d;
  return arg;
  }

cscript_argument make_cscript_argument(int64_t* p_i)
  {
  cscript_argument arg;
  arg.type = cscript_argument_type::cpp_pointer_to_int64;
  arg.value.i = (uint64_t)p_i;
  return arg;
  }

cscript_argument make_cscript_argument(uint64_t* p_i)
  {
  cscript_argument arg;
  arg.type = cscript_argument_type::cpp_pointer_to_int64;
  arg.value.i = (uint64_t)p_i;
  return arg;
  }

cscript_argument make_cscript_argument(double* p_d)
  {
  cscript_argument arg;
  arg.type = cscript_argument_type::cpp_pointer_to_double;
  arg.value.i = (uint64_t)p_d;
  return arg;
  }

namespace
  {
  external_function _convert_to_compiler(const cscript_external_function& f)
    {
    external_function e;
    e.func_ptr = f.func_ptr;
    e.name = f.name;
    switch (f.return_type)
      {
      case cscript_return_type::cpp_double:
        e.return_type = external_function_return_real;
        break;
      case cscript_return_type::cpp_int64:
        e.return_type = external_function_return_integer;
        break;
      case cscript_return_type::cpp_void:
        e.return_type = external_function_return_void;
        break;
      }
    for (const auto& arg : f.args)
      {
      external_function_parameter_type p;
      switch (arg)
        {
        case cscript_argument_type::cpp_int64:
          p = external_function_parameter_integer;
          break;
        case cscript_argument_type::cpp_double:
          p = external_function_parameter_real;
          break;
        case cscript_argument_type::cpp_pointer_to_int64:
          p = external_function_parameter_pointer_to_integer;
          break;
        case cscript_argument_type::cpp_pointer_to_double:
          p = external_function_parameter_pointer_to_real;
          break;
        }
      e.args.push_back(p);
      }
    return e;
    }

  VM::external_function _convert_to_vm(const cscript_external_function& f)
    {
    VM::external_function e;
    e.address = (uint64_t)f.func_ptr;
    e.name = f.name;
    switch (f.return_type)
      {
      case cscript_return_type::cpp_double:
        e.return_type = VM::external_function::T_DOUBLE;
        break;
      case cscript_return_type::cpp_int64:
        e.return_type = VM::external_function::T_INT64;
        break;
      case cscript_return_type::cpp_void:
        e.return_type = VM::external_function::T_VOID;
        break;
      }
    for (const auto& arg : f.args)
      {
      VM::external_function::argtype p;
      switch (arg)
        {
        case cscript_argument_type::cpp_int64:
          p = VM::external_function::T_INT64;
          break;
        case cscript_argument_type::cpp_double:
          p = VM::external_function::T_DOUBLE;
          break;
        case cscript_argument_type::cpp_pointer_to_int64:
          p = VM::external_function::T_CHAR_POINTER;
          break;
        case cscript_argument_type::cpp_pointer_to_double:
          p = VM::external_function::T_CHAR_POINTER;
          break;
        }
      e.arguments.push_back(p);
      }
    return e;
    }
  }  

std::unique_ptr<cscript_function> cscript_function::create(const std::string& script, cscript_environment& env, std::string& error_message, const std::vector<cscript_external_function>& externals)
  {
  if (script.empty())
    return std::unique_ptr<cscript_function>(nullptr);
  error_message = std::string();

  std::vector<external_function> externals_compiler;
  for (const auto& e : externals)
    {
    externals_compiler.push_back(_convert_to_compiler(e));
    }
  std::vector<VM::external_function> vm_externals;
  for (const auto& e : externals)
    {
    vm_externals.push_back(_convert_to_vm(e));
    }

  VM::vmcode code;
  try
    {
    auto tok = tokenize(script);
    auto prog = make_program(tok);
    optimize(prog);
    if (prog.statements.empty())
      return std::unique_ptr<cscript_function>(nullptr);
    compile(code, env.env, prog, externals_compiler);
    peephole_optimization(code);
    }
  catch (std::logic_error e)
    {
    error_message = e.what();
    return std::unique_ptr<cscript_function>(nullptr);
    }
  catch (std::runtime_error e)
    {
    error_message = e.what();
    return std::unique_ptr<cscript_function>(nullptr);
    }
  uint64_t size;
  uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
  return std::unique_ptr<cscript_function>(new cscript_function(f, size, vm_externals));
  }

std::unique_ptr<cscript_function> cscript_function::create(const std::string& script, cscript_environment& env, std::string& error_message)
  {
  std::vector<cscript_external_function> externals;
  return cscript_function::create(script, env, error_message, externals);
  }

cscript_function::cscript_function(uint8_t* bytecode, uint64_t bytecode_size, const std::vector<VM::external_function>& vm_externals) : _bytecode(bytecode), _bytecode_size(bytecode_size), _vm_externals(vm_externals)
  {
  }

cscript_function::~cscript_function()
  {
  VM::free_bytecode(_bytecode, _bytecode_size);
  }

double cscript_function::run(const std::vector<cscript_argument>& args, cscript_environment& env)
  {
  for (size_t i = 0; i < args.size(); ++i)
    {
    switch (i)
      {
      case 0:
        if (args[i].type == cscript_argument_type::cpp_double)
          env.reg.xmm0 = args[i].value.d;
        else
          env.reg.rcx = args[i].value.i;
        break;
      case 1:
        if (args[i].type == cscript_argument_type::cpp_double)
          env.reg.xmm1 = args[i].value.d;
        else
          env.reg.rdx = args[i].value.i;
        break;
      case 2:
        if (args[i].type == cscript_argument_type::cpp_double)
          env.reg.xmm2 = args[i].value.d;
        else
          env.reg.r8 = args[i].value.i;
        break;
      case 3:
        if (args[i].type == cscript_argument_type::cpp_double)
          env.reg.xmm3 = args[i].value.d;
        else
          env.reg.r9 = args[i].value.i;
        break;
      default:
        uint64_t* mem = (uint64_t*)env.reg.rsp;
        *(mem - 1) = args[i].value.i;
        env.reg.rsp -= 8;
        break;
      }
    }
  VM::run_bytecode(_bytecode, _bytecode_size, env.reg, _vm_externals);
  return env.reg.xmm0;
  }

bool make_cscript_global_variable(const std::string& variable_name, double variable_value, cscript_environment& env)
  {
  if (variable_name.empty() || variable_name.front() != '$')
    return false;
  global_variable_type new_global;
  new_global.address = env.env.global_var_offset;
  new_global.vt = global_value_real;
  env.env.globals.insert(std::make_pair(variable_name, new_global));
  env.reg.stack[env.env.global_var_offset / 8] = *(const uint64_t*)(&variable_value);
  env.env.global_var_offset += 8;
  return true;
  }

bool make_cscript_global_variable(const std::string& variable_name, int64_t variable_value, cscript_environment& env)
  {
  if (variable_name.empty() || variable_name.front() != '$')
    return false;
  global_variable_type new_global;
  new_global.address = env.env.global_var_offset;
  new_global.vt = global_value_integer;
  env.env.globals.insert(std::make_pair(variable_name, new_global));
  env.reg.stack[env.env.global_var_offset / 8] = (uint64_t)variable_value;
  env.env.global_var_offset += 8;
  return true;
  }

cscript_external_function make_external_function(const std::string& name, void* func_ptr, cscript_return_type return_type, const std::vector<cscript_argument_type>& arguments)
  {
  cscript_external_function f;
  f.name = name;
  f.func_ptr = func_ptr;
  f.return_type = return_type;
  f.args = arguments;
  return f;
  }

cscript_external_function make_external_function(const std::string& name, void* func_ptr, cscript_return_type return_type)
  {
  std::vector<cscript_argument_type> arguments;
  return make_external_function(name, func_ptr, return_type, arguments);
  }

cscript_external_function make_external_function(const std::string& name, void* func_ptr, cscript_return_type return_type, cscript_argument_type arg1)
  {
  std::vector<cscript_argument_type> arguments{ { arg1 } };
  return make_external_function(name, func_ptr, return_type, arguments);
  }

cscript_external_function make_external_function(const std::string& name, void* func_ptr, cscript_return_type return_type, cscript_argument_type arg1, cscript_argument_type arg2)
  {
  std::vector<cscript_argument_type> arguments{ { arg1, arg2 } };
  return make_external_function(name, func_ptr, return_type, arguments);
  }

cscript_external_function make_external_function(const std::string& name, void* func_ptr, cscript_return_type return_type, cscript_argument_type arg1, cscript_argument_type arg2, cscript_argument_type arg3)
  {
  std::vector<cscript_argument_type> arguments{ { arg1, arg2, arg3 } };
  return make_external_function(name, func_ptr, return_type, arguments);
  }

cscript_external_function make_external_function(const std::string& name, void* func_ptr, cscript_return_type return_type, cscript_argument_type arg1, cscript_argument_type arg2, cscript_argument_type arg3, cscript_argument_type arg4)
  {
  std::vector<cscript_argument_type> arguments{ { arg1, arg2, arg3, arg4 } };
  return make_external_function(name, func_ptr, return_type, arguments);
  }

COMPILER_END