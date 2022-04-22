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

std::unique_ptr<cscript_function> cscript_function::create(const std::string& script, cscript_environment& env, std::string& error_message)
  {
  if (script.empty())
    return std::unique_ptr<cscript_function>(nullptr);
  error_message = std::string();
  VM::vmcode code;
  try
    {
    auto tok = tokenize(script);
    auto prog = make_program(tok);
    optimize(prog);
    if (prog.statements.empty())
      return std::unique_ptr<cscript_function>(nullptr);
    compile(code, env.env, prog);
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
  return std::unique_ptr<cscript_function>(new cscript_function(f, size));
  }

cscript_function::cscript_function(uint8_t* bytecode, uint64_t bytecode_size) : _bytecode(bytecode), _bytecode_size(bytecode_size)
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
  VM::run_bytecode(_bytecode, _bytecode_size, env.reg);
  return env.reg.xmm0;
  }

COMPILER_END