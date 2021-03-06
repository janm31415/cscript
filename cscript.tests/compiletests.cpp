///////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////

#include "vm/vm.h"

#include "cscript/compiler.h"
#include "cscript/optimize.h"
#include "cscript/parse.h"
#include "cscript/tokenize.h"
#include "cscript/peephole.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <cmath>

#include "test_assert.h"

COMPILER_BEGIN

struct compile_fixture
  {
  typedef float(__cdecl* fun_ptr)(...);

  environment env;
  VM::registers reg;

  std::vector<external_function> empty_externals;

  compile_fixture()
    {
    }
  ~compile_fixture()
    {
    }
  VM::vmcode get_vmcode(const std::string& script, bool _optimize, bool _peephole, bool use_all_variable_registers, const std::vector<external_function>& externals)
    {
    auto tokens = tokenize(script);
    auto prog = make_program(tokens);

    if (use_all_variable_registers)
      {
      // this script makes sure that all registers are used for "unused" variables.
      // the actual script will thus use variables on the stack.
      std::string use_var_script = R"(()
int ___a, ___b, ___c, ___d, ___e, ___f, ___g, ___h, ___i, ___j, ___k;
float ___A, ___B, ___C, ___D, ___E, ___F, ___G, ___H, ___I, ___J, ___K;
)";
      auto tokens2 = tokenize(use_var_script);
      auto prog2 = make_program(tokens2);
      prog.statements.insert(prog.statements.begin(), prog2.statements.begin(), prog2.statements.end());
      }

    if (_optimize)
      optimize(prog);
    VM::vmcode code;
    compile(code, env, prog, externals);
    if (_peephole)
      peephole_optimization(code);
    if (false)
      {
      code.stream(std::cout);
      }

    return code;
    }

  double run(const std::string& script, bool _optimize = true, bool _peephole = true, bool use_all_variable_registers = false)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize, _peephole, use_all_variable_registers, empty_externals);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    try {
      VM::run_bytecode(f, size, reg);
      res = reg.xmm0;
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);

    return res;
    }

  double run_with_externals(const std::string& script, bool _optimize, bool _peephole, bool use_all_variable_registers, const std::vector<external_function>& externals)
    {
    std::vector<VM::external_function> vm_externals;
    for (const auto& f : externals)
      {
      VM::external_function fie;
      fie.address = (uint64_t)f.func_ptr;
      fie.name = f.name;
      switch (f.return_type)
        {
        case external_function_return_type::external_function_return_real:
          fie.return_type = VM::external_function::T_DOUBLE;
          break;
        case external_function_return_type::external_function_return_integer:
          fie.return_type = VM::external_function::T_INT64;
          break;
        default:
          fie.return_type = VM::external_function::T_VOID;
          break;
        }
      for (const auto& a : f.args)
        {
        VM::external_function::argtype arg;
        switch (a)
          {
          case external_function_parameter_type::external_function_parameter_integer:
            arg = VM::external_function::T_INT64;
            break;
          case external_function_parameter_type::external_function_parameter_real:
            arg = VM::external_function::T_DOUBLE;
            break;
          case external_function_parameter_type::external_function_parameter_pointer_to_real:
            arg = VM::external_function::T_CHAR_POINTER;
            break;
          case external_function_parameter_type::external_function_parameter_pointer_to_integer:
            arg = VM::external_function::T_CHAR_POINTER;
            break;
          }
        fie.arguments.push_back(arg);
        }
      vm_externals.push_back(fie);
      }
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize, _peephole, use_all_variable_registers, externals);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    try {
      VM::run_bytecode(f, size, reg, vm_externals);
      res = reg.xmm0;
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);

    return res;
    }

  double runi(const std::string& script, int64_t i1, bool _optimize = true, bool _peephole = true, bool use_all_variable_registers = false)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize, _peephole, use_all_variable_registers, empty_externals);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    reg.rcx = i1;
    try {
      VM::run_bytecode(f, size, reg);
      res = reg.xmm0;
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);
    return res;
    }

  double runii(const std::string& script, int64_t i1, int64_t i2, bool _optimize = true, bool _peephole = true, bool use_all_variable_registers = false)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize, _peephole, use_all_variable_registers, empty_externals);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    reg.rcx = i1;
    reg.rdx = i2;
    try {
      VM::run_bytecode(f, size, reg);
      res = reg.xmm0;
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);
    return res;
    }

  double runiii(const std::string& script, int64_t i1, int64_t i2, int64_t i3, bool _optimize = true, bool _peephole = true, bool use_all_variable_registers = false)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize, _peephole, use_all_variable_registers, empty_externals);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    reg.rcx = i1;
    reg.rdx = i2;
    reg.r8 = i3;
    try {
      VM::run_bytecode(f, size, reg);
      res = reg.xmm0;
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);
    return res;
    }

  double runiiii(const std::string& script, int64_t i1, int64_t i2, int64_t i3, int64_t i4, bool _optimize = true, bool _peephole = true, bool use_all_variable_registers = false)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize, _peephole, use_all_variable_registers, empty_externals);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    reg.rcx = i1;
    reg.rdx = i2;
    reg.r8 = i3;
    reg.r9 = i4;
    try {
      VM::run_bytecode(f, size, reg);
      res = reg.xmm0;
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);
    return res;
    }

  double runiiiii(const std::string& script, int64_t i1, int64_t i2, int64_t i3, int64_t i4, int64_t i5, bool _optimize = true, bool _peephole = true, bool use_all_variable_registers = false)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize, _peephole, use_all_variable_registers, empty_externals);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    reg.rcx = i1;
    reg.rdx = i2;
    reg.r8 = i3;
    reg.r9 = i4;
    uint64_t* mem = (uint64_t*)reg.rsp;
    *(mem - 1) = i5;
    reg.rsp -= 8;
    try {
      VM::run_bytecode(f, size, reg);
      res = reg.xmm0;
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);
    return res;
    }

  double runf(const std::string& script, double i1, bool _optimize = true, bool _peephole = true, bool use_all_variable_registers = false)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize, _peephole, use_all_variable_registers, empty_externals);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    reg.xmm0 = i1;
    try {
      VM::run_bytecode(f, size, reg);
      res = reg.xmm0;
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);
    return res;
    }

  double runff(const std::string& script, double i1, double i2, bool _optimize = true, bool _peephole = true, bool use_all_variable_registers = false)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize, _peephole, use_all_variable_registers, empty_externals);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    reg.xmm0 = i1;
    reg.xmm1 = i2;
    try {
      VM::run_bytecode(f, size, reg);
      res = reg.xmm0;
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);
    return res;
    }

  double runfff(const std::string& script, double i1, double i2, double i3, bool _optimize = true, bool _peephole = true, bool use_all_variable_registers = false)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize, _peephole, use_all_variable_registers, empty_externals);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    reg.xmm0 = i1;
    reg.xmm1 = i2;
    reg.xmm2 = i3;
    try {
      VM::run_bytecode(f, size, reg);
      res = reg.xmm0;
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);
    return res;
    }

  double runffff(const std::string& script, double i1, double i2, double i3, double i4, bool _optimize = true, bool _peephole = true, bool use_all_variable_registers = false)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize, _peephole, use_all_variable_registers, empty_externals);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    reg.xmm0 = i1;
    reg.xmm1 = i2;
    reg.xmm2 = i3;
    reg.xmm3 = i4;
    try {
      VM::run_bytecode(f, size, reg);
      res = reg.xmm0;
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);
    return res;
    }

  double runfffff(const std::string& script, double i1, double i2, double i3, double i4, double i5, bool _optimize = true, bool _peephole = true, bool use_all_variable_registers = false)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize, _peephole, use_all_variable_registers, empty_externals);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    reg.xmm0 = i1;
    reg.xmm1 = i2;
    reg.xmm2 = i3;
    reg.xmm3 = i4;
    uint64_t* mem = (uint64_t*)reg.rsp;
    *(mem - 1) = *reinterpret_cast<uint64_t*>(&i5);
    reg.rsp -= 8;
    try {
      VM::run_bytecode(f, size, reg);
      res = reg.xmm0;
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);
    return res;
    }

  double runififif(const std::string& script, int64_t i1, double f2, int64_t i3, double f4, int64_t i5, double f6, bool _optimize = true, bool _peephole = true, bool use_all_variable_registers = false)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize, _peephole, use_all_variable_registers, empty_externals);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    reg.rcx = i1;
    reg.xmm1 = f2;
    reg.r8 = i3;
    reg.xmm3 = f4;
    uint64_t* mem = (uint64_t*)reg.rsp;
    *(mem - 1) = i5;
    *(mem - 2) = *reinterpret_cast<uint64_t*>(&f6);
    reg.rsp -= 16;
    try {
      VM::run_bytecode(f, size, reg);
      res = reg.xmm0;
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);
    return res;
    }

  double runpi(const std::string& script, int64_t* i1, bool _optimize = true, bool _peephole = true, bool use_all_variable_registers = false)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize, _peephole, use_all_variable_registers, empty_externals);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    reg.rcx = (uint64_t)i1;
    try {
      VM::run_bytecode(f, size, reg);
      res = reg.xmm0;
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);
    return res;
    }

  double runpii(const std::string& script, int64_t* i1, int64_t i2, bool _optimize = true, bool _peephole = true, bool use_all_variable_registers = false)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize, _peephole, use_all_variable_registers, empty_externals);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    reg.rcx = (uint64_t)i1;
    reg.rdx = (uint64_t)i2;
    try {
      VM::run_bytecode(f, size, reg);
      res = reg.xmm0;
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);
    return res;
    }

  double runpipii(const std::string& script, int64_t* i1, int64_t* i2, int64_t i3, bool _optimize = true, bool _peephole = true, bool use_all_variable_registers = false)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize, _peephole, use_all_variable_registers, empty_externals);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    reg.rcx = (uint64_t)i1;
    reg.rdx = (uint64_t)i2;
    reg.r8 = (uint64_t)i3;
    try {
      VM::run_bytecode(f, size, reg);
      res = reg.xmm0;
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);
    return res;
    }

  double runpfpii(const std::string& script, double* i1, int64_t* i2, int64_t i3, bool _optimize = true, bool _peephole = true, bool use_all_variable_registers = false)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize, _peephole, use_all_variable_registers, empty_externals);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    reg.rcx = (uint64_t)i1;
    reg.rdx = (uint64_t)i2;
    reg.r8 = (uint64_t)i3;
    try {
      VM::run_bytecode(f, size, reg);
      res = reg.xmm0;
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);
    return res;
    }

  double runpf(const std::string& script, double* f1, bool _optimize = true, bool _peephole = true, bool use_all_variable_registers = false)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize, _peephole, use_all_variable_registers, empty_externals);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    reg.rcx = (uint64_t)f1;
    try {
      VM::run_bytecode(f, size, reg);
      res = reg.xmm0;
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);
    return res;
    }
  };

struct compiler_float : public compile_fixture
  {

  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ(1.0, run("()1.f;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(3.14, run("()3.14;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(-9.0, run("()3.0*(2/0.5-7);", optimize, peephole, use_all_variable_registers));
    TEST_EQ(9.0, run("()-3.0*(2/0.5-7);", optimize, peephole, use_all_variable_registers));
    }
  };

struct compiler_int : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ(1.0, run("()1;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(110.0, run("()110;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(135.0, run("()110+25;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(-135.0, run("()-(110+25);", optimize, peephole, use_all_variable_registers));
    }
  };

struct compiler_expr : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ(5.5, run("()3.0+2.5;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(-9.0, run("()3.0*(2/0.5-7);", optimize, peephole, use_all_variable_registers));
    TEST_EQ(9.0, run("()-3.0*(2/0.5-7);", optimize, peephole, use_all_variable_registers));
    TEST_EQ(55.0, run("()1.0 + 2.0 + 3.0 + 4.0 + 5.0 + 6.0 + 7.0 + 8.0 + 9.0 + 10.0;", optimize, peephole, use_all_variable_registers));
    TEST_ASSERT(55.0 == run("()1.0 + (2.0 + (3.0 + (4.0 + (5.0 + (6.0 + (7.0 + (8.0 + (9.0 + 10.0))))))));", optimize, peephole, use_all_variable_registers));
    TEST_ASSERT(55.0 == run("()1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10;", optimize, peephole, use_all_variable_registers));
    TEST_ASSERT(55.0 == run("()1 + (2 + (3 + (4 + (5 + (6 + (7 + (8 + (9 + 10))))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(-53.0, run("()1.0 - 2.0 - 3.0 - 4.0 - 5.0 - 6.0 - 7.0 - 8.0 - 9.0 - 10.0;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(-5.0, run("()1.0 - (2.0 - (3.0 - (4.0 - (5.0 - (6.0 - (7.0 - (8.0 - (9.0 - 10.0))))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(-53.0, run("()1 - 2 - 3 - 4 - 5 - 6 - 7 - 8 - 9 - 10;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(-5.0, run("()1 - (2 - (3 - (4 - (5 - (6 - (7 - (8 - (9 - 10))))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(3628800.0, run("()1.0 * 2.0 * 3.0 * 4.0 * 5.0 * 6.0 * 7.0 * 8.0 * 9.0 * 10.0;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(3628800.0, run("()1.0 * (2.0 * (3.0 * (4.0 * (5.0 * (6.0 * (7.0 * (8.0 * (9.0 * 10.0))))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(3628800.0, run("()1 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9 * 10;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(3628800.0, run("()1 * (2 * (3 * (4 * (5 * (6 * (7 * (8 * (9 * 10))))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ_CLOSE(2.75573200e-07, run("()1.0 / 2.0 / 3.0 / 4.0 / 5.0 / 6.0 / 7.0 / 8.0 / 9.0 / 10.0;", optimize, peephole), 1e-7);
    TEST_EQ(1, run("()5 / 3;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(2, run("()6 / 3;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(2, run("()8 / 2 / 2;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(32, run("()1024 / 2 / 2 / 2 / 2 / 2;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(32, run("() ((((1024 / 2) / 2) / 2) / 2) / 2;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(8, run("()1024 / (512 / (256 / (128 / (64 / 32))));", optimize, peephole, use_all_variable_registers));
    }
  };

struct compiler_relop : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ(0, run("()5.0 < 3.0;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()3.0 < 5.0;", optimize, peephole, use_all_variable_registers));

    TEST_EQ(0, run("()5 < 3;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()3 < 5;", optimize, peephole, use_all_variable_registers));

    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(5.0 < 3.0)))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(3.0 < 5.0)))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 < 3.0))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 < 5.0))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 < 3.0)))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 < 5.0)))))));", optimize, peephole, use_all_variable_registers));

    TEST_EQ(1, run("()(1 * (3 < 5));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1 * (1 * (3 < 5)));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1 * (5 < 3));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1 * (1 * (5 < 3)));", optimize, peephole, use_all_variable_registers));


    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(5.0 > 3.0)))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(3.0 > 5.0)))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 > 3.0))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 > 5.0))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 > 3.0)))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 > 5.0)))))));", optimize, peephole, use_all_variable_registers));

    TEST_EQ(1, run("()5 > 3;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()3 > 5;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1 * (3 > 5));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1 * (1 * (3 > 5)));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1 * (5 > 3));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1 * (1 * (5 > 3)));", optimize, peephole, use_all_variable_registers));

    TEST_EQ(0, run("()5.0 <= 3.0;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()3.0 <= 5.0;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(5.0 <= 3.0)))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(3.0 <= 5.0)))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 <= 3.0))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 <= 5.0))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 <= 3.0)))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 <= 5.0)))))));", optimize, peephole, use_all_variable_registers));

    TEST_EQ(0, run("()5 <= 3;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()3 <= 5;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1 * (3 <= 5));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1 * (1 * (3 <= 5)));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1 * (5 <= 3));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1 * (1 * (5 <= 3)));", optimize, peephole, use_all_variable_registers));

    TEST_EQ(1, run("()5 >= 3;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()3 >= 5;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1 * (3 >= 5));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1 * (1 * (3 >= 5)));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1 * (5 >= 3));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1 * (1 * (5 >= 3)));", optimize, peephole, use_all_variable_registers));

    TEST_EQ(1, run("()5.0 >= 3.0;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()3.0 >= 5.0;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(5.0 >= 3.0)))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(3.0 >= 5.0)))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 >= 3.0))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 >= 5.0))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 >= 3.0)))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 >= 5.0)))))));", optimize, peephole, use_all_variable_registers));

    TEST_EQ(0, run("()5 == 3;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()3 == 3;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1 * (3 == 5));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1 * (1 * (3 == 5)));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1 * (3 == 3));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1 * (1 * (3 == 3)));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()5.0 == 3.0;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()3.0 == 3.0;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(5.0 == 3.0)))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(3.0 == 3.0)))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 == 3.0))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 == 3.0))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 == 3.0)))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 == 3.0)))))));", optimize, peephole, use_all_variable_registers));

    TEST_EQ(1, run("()5 != 3;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()3 != 3;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1 * (3 != 5));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1 * (1 * (3 != 5)));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1 * (3 != 3));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1 * (1 * (3 != 3)));", optimize, peephole, use_all_variable_registers));

    TEST_EQ(1, run("()5.0 != 3.0;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()3.0 != 3.0;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(5.0 != 3.0)))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(3.0 != 3.0)))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 != 3.0))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 != 3.0))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 != 3.0)))))));", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 != 3.0)))))));", optimize, peephole, use_all_variable_registers));
    }
  };

struct compiler_integer_var : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ(3, run("() int i = 3; i;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1512, run("() int i = 3*7*8*9; i;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(3, run("() int i = 3.0; i;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1512, run("() int i = 3.0*7.0*8.0*9.0; i;", optimize, peephole, use_all_variable_registers));
    }
  };

struct compiler_float_var : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ(3, run("() float f = 3.0; f;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1512, run("() float f = 3.0*7.0*8.0*9.0; f;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(3, run("() float f = 3; f;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(1512, run("() float f = 3*7*8*9; f;", optimize, peephole, use_all_variable_registers));
    }
  };

struct compiler_assignment : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ(4.1415000000000006, run("() float f; int i; f = 3.1415; i = 1; f+i;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(4.1415000000000006, run("() float f; int i; f = 3.1415; i = 1; f+=i;f;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(4.0, run("() float f; int i; f = 3.1415; i = 1; i+=f;i;", optimize, peephole, use_all_variable_registers));

    TEST_EQ(2.1415, run("() float f; int i; f = 3.1415; i = 1; f-=i;f;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(-2.0, run("() float f; int i; f = 3.1415; i = 1; i-=f;i;", optimize, peephole, use_all_variable_registers));

    TEST_EQ(6.2830, run("() float f; int i; f = 3.1415; i = 2; f*=i;f;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(6., run("() float f; int i; f = 3.1415; i = 2; i*=f;i;", optimize, peephole, use_all_variable_registers));

    TEST_EQ(1.5707500000000001, run("() float f; int i; f = 3.1415; i = 2; f/=i;f;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(3.0, run("() float f; int i; f = 3.1415; i = 9; i/=f;i;", optimize, peephole, use_all_variable_registers));

    TEST_EQ(4.0, run("() int i = 9; i/=(1+1);i;", optimize, peephole, use_all_variable_registers));
    }
  };

struct compiler_array : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ(6.0, run("() float f[3];\nf[0] = 1;\nf[1] = 2;\nf[2] = 3;\nf[0]*f[1]*f[2];", optimize, peephole, use_all_variable_registers));
    TEST_EQ(6.0, run("() int f[3];\nf[0] = 1;\nf[1] = 2;\nf[2] = 3;\nf[0]*f[1]*f[2];", optimize, peephole, use_all_variable_registers));

    TEST_EQ(1.0 + 3.14, run("() float f[10];\nf[8] = 1;\nf[8] += 3.14;f[8];", optimize, peephole, use_all_variable_registers));
    TEST_EQ(4.0, run("() int f[10];\nf[8] = 1;\nf[8] += 3;f[8];", optimize, peephole, use_all_variable_registers));

    TEST_EQ(1.0 - 3.14, run("() float f[10];\nf[8] = 1;\nf[8] -= 3.14;f[8];", optimize, peephole, use_all_variable_registers));
    TEST_EQ(-2.0, run("() int f[10];\nf[8] = 1;\nf[8] -= 3;f[8];", optimize, peephole, use_all_variable_registers));

    TEST_EQ(6.28, run("() float f[10];\nf[8] = 2;\nf[8] *= 3.14;f[8];", optimize, peephole, use_all_variable_registers));
    TEST_EQ(6.0, run("() int f[10];\nf[8] = 2;\nf[8] *= 3;f[8];", optimize, peephole, use_all_variable_registers));

    TEST_EQ(15.8 / 2.15, run("() float f[10];\nf[8] = 15.8;\nf[8] /= 2.15;f[8];", optimize, peephole, use_all_variable_registers));
    TEST_EQ(3.0, run("() int f[10];\nf[8] = 10;\nf[8] /= 3;f[8];", optimize, peephole, use_all_variable_registers));
    }
  };

struct comment_test : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    std::string script =
      R"( ()
int i[3]; // make an array of 3 integers
i[1] = 8; // assign 8 to index 1 position
i[1] /= 2; // divide index 1 position by 2
i[1]; // return the result in index 1
    )";

    TEST_EQ(4, run(script, optimize, peephole, use_all_variable_registers));

    script =
      R"(  ()
int i[3]; // make an array of 3 integers
i[1] = 8; // assign 8 to index 1 position
i[1];
/*
i[1] /= 2; // divide index 1 position by 2
i[1]; // return the result in index 1
*/
    )";

    TEST_EQ(8, run(script, optimize, peephole, use_all_variable_registers));
    }
  };

struct parameter_test : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ(4.f, runi("(int i) i + 1;", 3, optimize, peephole, use_all_variable_registers));
    TEST_EQ(10.f, runii("(int i, int j) i + j;", 3, 7, optimize, peephole, use_all_variable_registers));
    TEST_EQ(15.f, runiii("(int i, int j, int k) i + j + k;", 3, 7, 5, optimize, peephole, use_all_variable_registers));
    TEST_EQ(35.f, runiiii("(int i, int j, int k, int l) i + j + k + l;", 3, 7, 5, 20, optimize, peephole, use_all_variable_registers));
    TEST_EQ(135.f, runiiiii("(int i, int j, int k, int l, int m) i + j + k + l + m;", 3, 7, 5, 20, 100, optimize, peephole, use_all_variable_registers));

    TEST_EQ(1.2 + 1.5, runf("(float f) f + 1.5;", 1.2, optimize, peephole, use_all_variable_registers));
    TEST_EQ(1.2 + 1.5, runff("(float f, float g) f + g;", 1.2, 1.5, optimize, peephole, use_all_variable_registers));
    TEST_EQ(1.2 + 1.5 + 7.9, runfff("(float f, float g, float h) f + g + h;", 1.2, 1.5, 7.9, optimize, peephole, use_all_variable_registers));
    TEST_EQ(1.2 + 1.5 + 7.9 + 10.4, runffff("(float f, float g, float h, float i) f + g + h + i;", 1.2, 1.5, 7.9, 10.4, optimize, peephole, use_all_variable_registers));
    TEST_EQ(1.2 + 1.5 + 7.9 + 10.4 + 19.1, runfffff("(float f, float g, float h, float i, float j) f + g + h + i + j;", 1.2, 1.5, 7.9, 10.4, 19.1, optimize, peephole, use_all_variable_registers));


    TEST_EQ(1.0 + 1.5 + 7.0 + 10.4 + 19.0 + 3.14, runififif("(int i1, float f2, int i3, float f4, int i5, float f6) i1+f2+i3+f4+i5+f6;", 1, 1.5, 7, 10.4, 19, 3.14, optimize, peephole, use_all_variable_registers));
    }
  };

struct parameter_pointer_test : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    int64_t i[3];
    double f[3];
    runpi("(int* i) i[0] = 0; i[1] = 1; i[2] = 2;", &i[0], optimize, peephole, use_all_variable_registers);
    TEST_EQ(0, i[0]);
    TEST_EQ(1, i[1]);
    TEST_EQ(2, i[2]);
    runpf("(float* f) f[0] = 0.5; f[1] = 1.5; f[2] = 2.5;", &f[0], optimize, peephole, use_all_variable_registers);
    TEST_EQ(0.5, f[0]);
    TEST_EQ(1.5, f[1]);
    TEST_EQ(2.5, f[2]);
    runpi("(int* i) i[0] += 7; i[1] += 4; i[2] += 2;", &i[0], optimize, peephole, use_all_variable_registers);
    TEST_EQ(7, i[0]);
    TEST_EQ(5, i[1]);
    TEST_EQ(4, i[2]);
    runpf("(float* f) f[0] += 0.1; f[1] += 0.2; f[2] += 0.3;", &f[0], optimize, peephole, use_all_variable_registers);
    TEST_EQ(0.5 + 0.1, f[0]);
    TEST_EQ(1.5 + 0.2, f[1]);
    TEST_EQ(2.5 + 0.3, f[2]);
    runpi("(int* i) i[0] -= 10; i[1] -= 4; i[2] -= 21;", &i[0], optimize, peephole, use_all_variable_registers);
    TEST_EQ(-3, i[0]);
    TEST_EQ(1, i[1]);
    TEST_EQ(-17, i[2]);
    runpf("(float* f) f[0] -= 0.1; f[1] -= 0.2; f[2] -= 0.3;", &f[0], optimize, peephole, use_all_variable_registers);
    TEST_EQ(0.5, f[0]);
    TEST_EQ(1.5, f[1]);
    TEST_EQ(2.5, f[2]);
    runpi("(int* i) i[0] *= 2;", &i[0], optimize, peephole, use_all_variable_registers);
    TEST_EQ(-6, i[0]);
    TEST_EQ(1, i[1]);
    TEST_EQ(-17, i[2]);
    runpf("(float* f) f[1] *= 3.14f;", &f[0], optimize, peephole, use_all_variable_registers);
    TEST_EQ(0.5, f[0]);
    TEST_EQ(1.5 * 3.14, f[1]);
    TEST_EQ(2.5, f[2]);
    runpi("(int* i) i[0] /= 2;", &i[0], optimize, peephole, use_all_variable_registers);
    TEST_EQ(-3, i[0]);
    TEST_EQ(1, i[1]);
    TEST_EQ(-17, i[2]);
    runpf("(float* f) f[1] /= 3.14f;", &f[0], optimize, peephole, use_all_variable_registers);
    TEST_EQ(0.5, f[0]);
    TEST_EQ(1.5, f[1]);
    TEST_EQ(2.5, f[2]);
    }
  };

struct parameter_dereference_test : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    double f = 3.14;
    int64_t i = 5;
    TEST_EQ(3.14, runpf("(float* f) float g = *f; g;", &f, optimize, peephole, use_all_variable_registers));
    TEST_EQ(5, runpi("(int* i) int g = *i; g;", &i, optimize, peephole, use_all_variable_registers));
    TEST_EQ(10.14, runpf("(float* f) float g = (1 + (1 + (1 + (1 + (1 + (1 + ( 1 + *f))))))); g;", &f, optimize, peephole, use_all_variable_registers));
    TEST_EQ(12, runpi("(int* i) int g = (1 + (1 + (1 + (1 + (1 + (1 + ( 1 + *i))))))); g;", &i, optimize, peephole, use_all_variable_registers));

    TEST_EQ(3.14, runpf("(float* f) float g = f[0]; g;", &f, optimize, peephole, use_all_variable_registers));
    TEST_EQ(5, runpi("(int* i) int g = i[0]; g;", &i, optimize, peephole, use_all_variable_registers));
    TEST_EQ(10.14, runpf("(float* f) float g = (1 + (1 + (1 + (1 + (1 + (1 + ( 1 + f[0]))))))); g;", &f, optimize, peephole, use_all_variable_registers));
    TEST_EQ(12, runpi("(int* i) int g = (1 + (1 + (1 + (1 + (1 + (1 + ( 1 + i[0]))))))); g;", &i, optimize, peephole, use_all_variable_registers));

    TEST_EQ(3.14 * 3.14, runpf("(float* f) *f * *f;", &f, optimize, peephole, use_all_variable_registers));
    TEST_EQ(25.0, runpi("(int* i) *i * *i;", &i, optimize, peephole, use_all_variable_registers));

    runpf("(float* f) *f += 1.2;", &f, optimize, peephole);
    TEST_EQ(3.14 + 1.2, f);
    runpf("(float* f) *f = 1.2;", &f, optimize, peephole);
    TEST_EQ(1.2, f);
    runpf("(float* f) *f -= 1.4;", &f, optimize, peephole);
    TEST_EQ(1.2 - 1.4, f);
    runpf("(float* f) *f *= 2.8;", &f, optimize, peephole);
    TEST_EQ((1.2 - 1.4) * 2.8, f);
    runpf("(float* f) *f /= 2.5;", &f, optimize, peephole);
    TEST_EQ((1.2 - 1.4) * 2.8 / 2.5, f);

    runpi("(int* i) *i += 2;", &i, optimize, peephole);
    TEST_EQ(7, i);
    runpi("(int* i) *i = 2;", &i, optimize, peephole);
    TEST_EQ(2, i);
    runpi("(int* i) *i -= 9;", &i, optimize, peephole);
    TEST_EQ(-7, i);
    runpi("(int* i) *i *= 9;", &i, optimize, peephole);
    TEST_EQ(-63, i);
    runpi("(int* i) *i /= 6;", &i, optimize, peephole);
    TEST_EQ(-10, i);
    }
  };

struct inc_dec_test : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ(4, run("() int i = 3; ++i;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(2, run("() int i = 3; --i;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(4, run("() int i = 3; ++i; i;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(2, run("() int i = 3; --i; i;", optimize, peephole, use_all_variable_registers));

    TEST_EQ(4, run("() float i = 3; ++i;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(2, run("() float i = 3; --i;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(4, run("() float i = 3; ++i; i;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(2, run("() float i = 3; --i; i;", optimize, peephole, use_all_variable_registers));

    TEST_EQ(4, run("() int i[4]; i[2] = 3; ++i[2];", optimize, peephole, use_all_variable_registers));
    TEST_EQ(4, run("() int i[4]; i[2] = 3; ++i[2]; i[2];", optimize, peephole, use_all_variable_registers));
    TEST_EQ(2, run("() int i[4]; i[2] = 3; --i[2];", optimize, peephole, use_all_variable_registers));
    TEST_EQ(2, run("() int i[4]; i[2] = 3; --i[2]; i[2];", optimize, peephole, use_all_variable_registers));

    TEST_EQ(4, run("() float i[4]; i[2] = 3; ++i[2];", optimize, peephole, use_all_variable_registers));
    TEST_EQ(4, run("() float i[4]; i[2] = 3; ++i[2]; i[2];", optimize, peephole, use_all_variable_registers));
    TEST_EQ(2, run("() float i[4]; i[2] = 3; --i[2];", optimize, peephole, use_all_variable_registers));
    TEST_EQ(2, run("() float i[4]; i[2] = 3; --i[2]; i[2];", optimize, peephole, use_all_variable_registers));

    int64_t i[3];
    runpi("(int* i) i[0] = 0; i[1] = 3; i[2] = 6; ++i[1];", &i[0], optimize, peephole);
    TEST_EQ(0, i[0]);
    TEST_EQ(4, i[1]);
    TEST_EQ(6, i[2]);

    double f[3];
    runpf("(float* f) f[0] = 0; f[1] = 3; f[2] = 6; ++f[1];", &f[0], optimize, peephole);
    TEST_EQ(0.f, f[0]);
    TEST_EQ(4.f, f[1]);
    TEST_EQ(6.f, f[2]);

    int64_t i1 = 7;
    runpi("(int* i) ++*i;", &i1, optimize, peephole);
    TEST_EQ(8, i1);
    runpi("(int* i) --*i;", &i1, optimize, peephole);
    TEST_EQ(7, i1);

    double f1 = 7.5;
    runpf("(float* i) ++*i;", &f1, optimize, peephole);
    TEST_EQ(8.5f, f1);
    runpf("(float* i) --*i;", &f1, optimize, peephole);
    TEST_EQ(7.5f, f1);
    }
  };

struct for_loop_test : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ(1225.f, run("() float f = 0; for (int i = 0; i < 50; ++i) { f += i; } f;", optimize, peephole, use_all_variable_registers));
    }
  };

struct optimize_tests : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ(0, run("() int i = 0; i;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("() float f = 0.0; f;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0, run("() float f = 0; f;", optimize, peephole, use_all_variable_registers));
    }
  };

struct funccall_tests : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ(std::sqrt(2.0), run("() sqrt(2.f);", optimize, peephole, use_all_variable_registers));

    TEST_EQ(std::sin(0.5), run("() sin(0.5f);", optimize, peephole, use_all_variable_registers));

    TEST_EQ(std::cos(0.5), run("() cos(0.5f);", optimize, peephole, use_all_variable_registers));

    TEST_EQ(std::exp(0.5), run("() exp(0.5f);", optimize, peephole, use_all_variable_registers));

    TEST_EQ(std::log(0.5), run("() log(0.5f);", optimize, peephole, use_all_variable_registers));

    TEST_EQ(std::log2(0.5), run("() log2(0.5f);", optimize, peephole, use_all_variable_registers));

    TEST_EQ(std::fabs(-0.5), run("() fabs(0.5f);", optimize, peephole, use_all_variable_registers));

    TEST_EQ(std::tan(0.5), run("() tan(0.5f);", optimize, peephole, use_all_variable_registers));

    TEST_EQ(std::atan(0.5), run("() atan(0.5f);", optimize, peephole, use_all_variable_registers));

    TEST_EQ(std::atan2(0.5, 0.7), run("() atan2(0.5f,0.7);", optimize, peephole, use_all_variable_registers));

    TEST_EQ(std::pow(0.5, 0.7), run("() pow(0.5f,0.7);", optimize, peephole, use_all_variable_registers));

    TEST_EQ(std::min(0.5, 0.7), run("() min(0.5f,0.7);", optimize, peephole, use_all_variable_registers));

    TEST_EQ(std::max(0.5, 0.7), run("() max(0.5f,0.7);", optimize, peephole, use_all_variable_registers));
    }
  };

struct rsp_offset_test : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ_CLOSE(0.0, runf("(float x) float y = 1.0 - x; sin(y*y*3.141592653589793238462643383*4.0);", 1.f, optimize, peephole, use_all_variable_registers), 1e-5);
    TEST_EQ_CLOSE(0.0, runf("(float x) float y = 1.0 - x; sin(y*y*3.141592653589793238462643383*4.0);", 0.f, optimize, peephole, use_all_variable_registers), 1e-5);
    }
  };

struct modulo_test : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ(2.0, run("() 5%3;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0.14000000000000012, run("() 3.14%3;", optimize, peephole, use_all_variable_registers));
    }
  };

struct if_test : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ(7.0, run("() float f = 0; if (3 > 2) { f = 7; } else { f = 9; } f;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(9.0, run("() float f = 0; if (3 > 2) { f = 9; } f;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(9.0, run("() float f = 0; if (3 < 2) { f = 7; } else { f = 9; } f;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(0.0, run("() float f = 0; if (3 < 2) { f = 9; } f;", optimize, peephole, use_all_variable_registers));

    TEST_EQ(10.0, run("() float f = 0; if (3 < 2) { f = 9; } else if (3 > 2) { f = 10; } else { f = 11; } f;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(11.0, run("() float f = 0; if (3 < 2) { f = 9; } else if (3 == 2) { f = 10; } else { f = 11; } f;", optimize, peephole, use_all_variable_registers));
    }
  };

struct dead_code_test : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ(3.0, run("() float f = 0; int i; int j = 3; j;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(3.0, run("() float f; f = 0; int i; int j = 3; j;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(3.0, run("() float f; f += 1; int i; int j = 3; j;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(3.0, run("() float f; f -= 2; int i; int j = 3; j;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(3.0, run("() float f; f *= 4; int i; i = 2; int j = 3; j;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(3.0, run("() float f; f /= 7; int i; i += 5; int j = 3; j;", optimize, peephole, use_all_variable_registers));
    }
  };

struct array_assignment_test : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ(3.4, run("() float f[3] = {1.2, 3.4, 5.6}; f[1];", optimize, peephole, use_all_variable_registers));    
    TEST_EQ(34, run("() int i[3] = {12, 34, 56}; i[1];", optimize, peephole, use_all_variable_registers));
    if (optimize)
      {
      TEST_EQ(4.6, run("() float f[3] = {1.2, 3.4+1.2, 5.6}; f[1];", optimize, peephole, use_all_variable_registers));
      TEST_EQ(38, run("() int i[3] = {12, 34+4, 56}; i[1];", optimize, peephole, use_all_variable_registers));
      }
    }
  };

namespace
  {
  double simple_external_function()
    {
    return 5.8;
    }

  double simple_add_external_function(double a, double b)
    {
    return a+b;
    }
  }

struct external_function_test : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    external_function fie;
    fie.name = "external";
    fie.func_ptr = &simple_external_function;
    fie.return_type = external_function_return_real;
    std::vector<external_function> externals;
    externals.push_back(fie);
    TEST_EQ(5.8, run_with_externals("() external();", optimize, peephole, use_all_variable_registers, externals));

    fie.name = "add";
    fie.func_ptr = &simple_add_external_function;
    fie.return_type = external_function_return_real;    
    fie.args.push_back(external_function_parameter_real);
    fie.args.push_back(external_function_parameter_real);
    externals.push_back(fie);

    TEST_EQ(3.24, run_with_externals("() add(3.14, 0.1);", optimize, peephole, use_all_variable_registers, externals));

    }
  };

struct array_address_test : public compile_fixture
  {
  void test(bool optimize, bool peephole, bool /*use_all_variable_registers = false*/)
    {
    std::string script = R"((int* addr)
int my_array[5] = {1, 2, 3, 4, 5};
*addr = my_array;
)";
    int64_t addr;
    runpi(script, &addr, optimize, peephole, false);
    TEST_EQ((uint64_t)addr, this->reg.rsp-6*8);
    }
  };

struct harmonic : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    auto tic = std::chrono::high_resolution_clock::now();
    TEST_EQ_CLOSE(14.3927, run("() float sum = 0.0; for (int i = 1; i<1000000; ++i) { sum += 1.0/i; } sum;", optimize, peephole, use_all_variable_registers), 1e-4);
    auto toc = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(toc - tic).count();
    printf("Harmonic timing: %lldms\n", ms);
    }
  };

struct hamming : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    int max_size = 1000000;
    std::vector<int64_t> a, b;
    a.reserve(max_size);
    b.reserve(max_size);
    for (int i = 0; i < max_size; ++i)
      {
      a.push_back(i);
      b.push_back(i);
      }
    b[max_size / 2] = 100;
    b[max_size - 7] = 200;
    auto tic = std::chrono::high_resolution_clock::now();
    TEST_EQ(2, runpipii("(int* a, int* b, int size) int hamming = 0; for (int i = 0; i < size; ++i) { hamming += a[i] != b[i];} hamming;", a.data(), b.data(), max_size, optimize, peephole, use_all_variable_registers));
    auto toc = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(toc - tic).count();
    printf("Hamming distance timing: %lldms\n", ms);
    }
  };

struct fibonacci : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    auto tic = std::chrono::high_resolution_clock::now();
    TEST_EQ(102334155, runi("(int i) int a = 0; int b = 1; for (int j = 0; j < i; ++j) { int c = a+b; a = b; b = c; } a;", 40, optimize, peephole, use_all_variable_registers));
    auto toc = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(toc - tic).count();
    printf("Fibonacci timing: %lldms\n", ms);
    }
  };

struct quicksort : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    uint64_t max_size = 10000;
    uint32_t x = 0x76543513;
    std::vector<int64_t> a, st;
    a.resize(max_size);
    st.resize(max_size);
    for (auto& v : a)
      {
      x ^= x << 13;
      x ^= x >> 17;
      x ^= x << 5;
      v = x % max_size;
      }
    auto tic = std::chrono::high_resolution_clock::now();
    runpipii(R"((int* a, int* stack, int size)
      int lo = 0;
      int hi = size-1;
      // initialize top of stack
      int top = -1;
      // push initial values of l and h to stack
      stack[++top] = lo;
      stack[++top] = hi;
      while (top >= 0)
        {
        hi = stack[top];
        --top;
        lo = stack[top];
        --top;
        // partitioning algorithm
        // Set pivot element at its correct position
        // in sorted array
        int x = a[hi];
        int i = (lo - 1);
        for (int j = lo; j <= hi - 1; ++j)
          {
          if (a[j] <= x)
            {
            ++i;
            int tmp = a[i];
            a[i] = a[j];
            a[j] = tmp;
            }
          }
        int tmp2 = a[i+1];
        a[i+1] = a[hi];
        a[hi] = tmp2;
        int p = i+1;
        // end partitioning algorithm

        // If there are elements on left side of pivot,
        // then push left side to stack
        if (p - 1 > lo)
          {
          stack[++top] = lo;
          stack[++top] = p-1;
          }

        // If there are elements on right side of pivot,
        // then push right side to stack
        if (p + 1 < hi)
          {
          stack[++top] = p+1;
          stack[++top] = hi;
          }
        }
)", a.data(), st.data(), max_size, optimize, peephole, use_all_variable_registers);
    auto toc = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(toc - tic).count();
    printf("qsort timing: %lldms\n", ms);
    bool array_is_sorted = true;
    for (uint64_t i = 0; i < max_size - 1; ++i)
      array_is_sorted &= a[i] <= a[i + 1];
    TEST_ASSERT(array_is_sorted);
    }
  };

struct quicksortdouble : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    uint64_t max_size = 10000;
    uint32_t x = 0x76543513;
    std::vector<double> a;
    std::vector<int64_t> st;
    a.resize(max_size);
    st.resize(max_size);
    for (auto& v : a)
      {
      x ^= x << 13;
      x ^= x >> 17;
      x ^= x << 5;
      v = (double)(x % max_size) / (double)max_size;
      }
    auto tic = std::chrono::high_resolution_clock::now();
    runpfpii(R"((float* a, int* stack, int size)
      int lo = 0;
      int hi = size-1;
      // initialize top of stack
      int top = -1;
      // push initial values of l and h to stack
      stack[++top] = lo;
      stack[++top] = hi;
      while (top >= 0)
        {
        hi = stack[top];
        --top;
        lo = stack[top];
        --top;
        // partitioning algorithm
        // Set pivot element at its correct position
        // in sorted array
        float x = a[hi];
        int i = (lo - 1);
        for (int j = lo; j <= hi - 1; ++j)
          {
          if (a[j] <= x)
            {
            ++i;
            float tmp = a[i];
            a[i] = a[j];
            a[j] = tmp;
            }
          }
        float tmp2 = a[i+1];
        a[i+1] = a[hi];
        a[hi] = tmp2;
        int p = i+1;
        // end partitioning algorithm

        // If there are elements on left side of pivot,
        // then push left side to stack
        if (p - 1 > lo)
          {
          stack[++top] = lo;
          stack[++top] = p-1;
          }

        // If there are elements on right side of pivot,
        // then push right side to stack
        if (p + 1 < hi)
          {
          stack[++top] = p+1;
          stack[++top] = hi;
          }
        }
)", a.data(), st.data(), max_size, optimize, peephole, use_all_variable_registers);
    auto toc = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(toc - tic).count();
    printf("qsort double timing: %lldms\n", ms);
    bool array_is_sorted = true;
    for (uint64_t i = 0; i < max_size - 1; ++i)
      array_is_sorted &= a[i] <= a[i + 1];
    TEST_ASSERT(array_is_sorted);
    }
  };

struct digitspi : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    uint64_t nr_terms = 10000;
    std::string script = R"((int nr_of_terms)
/* This method computes pi by using the power series expansion of 
   atan(x) = x - x^3/3 + x^5/5 - ... together with formulas like 
   pi = 16*atan(1/5) - 4*atan(1/239). 
   This gives about 1.4 decimals per term.
*/
    float x1 = 1.0/5.0;
    float x2 = 1.0/239.0;
    float pi = 0;
    for (int i = 0; i < nr_of_terms; ++i)
      {
      float sign = pow(-1.0, i);
      float power = 2*i+1;
      pi += 16*sign * pow(x1, power)/power - 4*sign * pow(x2, power)/power;
      }
    pi;
)";
    auto tic = std::chrono::high_resolution_clock::now();
    double result = runi(script, nr_terms, optimize, peephole, use_all_variable_registers);
    TEST_EQ_CLOSE(3.1415926535, result, 1e-8);
    auto toc = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(toc - tic).count();
    printf("digitspi timing: %lldms\n", ms);
    }
  };

struct test_strength_reduction : public compile_fixture
  {
  void test()
    {
    TEST_EQ(1, runii("(int a, int b) a <= b-1;", 2, 3));
    TEST_EQ(0, runii("(int a, int b) a <= b-1;", 2, 2));
    TEST_EQ(1, runii("(int a, int b) a <= -1+b;", 2, 3));
    TEST_EQ(0, runii("(int a, int b) a <= -1+b;", 2, 2));
    TEST_EQ(1, runii("(int a, int b) a+1 <= b;", 2, 3));
    TEST_EQ(0, runii("(int a, int b) a+1 <= b;", 2, 2));
    TEST_EQ(1, runii("(int a, int b) 1+a <= b;", 2, 3));
    TEST_EQ(0, runii("(int a, int b) 1+a <= b;", 2, 2));

    TEST_EQ(1, runii("(int a, int b) a >= b+1;", 3, 2));
    TEST_EQ(0, runii("(int a, int b) a >= b+1;", 2, 2));
    TEST_EQ(1, runii("(int a, int b) a >= 1+b;", 3, 2));
    TEST_EQ(0, runii("(int a, int b) a >= 1+b;", 2, 2));
    TEST_EQ(1, runii("(int a, int b) a-1 >= b;", 3, 2));
    TEST_EQ(0, runii("(int a, int b) a-1 >= b;", 2, 2));
    TEST_EQ(1, runii("(int a, int b) -1+a >= b;", 3, 2));
    TEST_EQ(0, runii("(int a, int b) -1+a >= b;", 2, 2));

    TEST_EQ(0, runii("(int a, int b) a < b+1;", 3, 2));
    TEST_EQ(1, runii("(int a, int b) a < b+1;", 3, 3));
    TEST_EQ(0, runii("(int a, int b) a < 1+b;", 3, 2));
    TEST_EQ(1, runii("(int a, int b) a < 1+b;", 3, 3));
    TEST_EQ(0, runii("(int a, int b) a-1 < b;", 3, 2));
    TEST_EQ(1, runii("(int a, int b) a-1 < b;", 3, 3));
    TEST_EQ(0, runii("(int a, int b) -1+a < b;", 3, 2));
    TEST_EQ(1, runii("(int a, int b) -1+a < b;", 3, 3));

    TEST_EQ(0, runii("(int a, int b) a > b-1;", 2, 3));
    TEST_EQ(1, runii("(int a, int b) a > b-1;", 3, 3));
    TEST_EQ(0, runii("(int a, int b) a > -1+b;", 2, 3));
    TEST_EQ(1, runii("(int a, int b) a > -1+b;", 3, 3));
    TEST_EQ(0, runii("(int a, int b) a+1 > b;", 2, 3));
    TEST_EQ(1, runii("(int a, int b) a+1 > b;", 3, 3));
    TEST_EQ(0, runii("(int a, int b) 1+a > b;", 2, 3));
    TEST_EQ(1, runii("(int a, int b) 1+a > b;", 3, 3));

    TEST_EQ(0, runff("(float a, float b) a <= b-1;", 2.8, 3.5));
    }
  };

struct test_many_variables : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    std::string script = R"(
()
int a=1,b=2,c=3,d=4,e=5,f=6,g=7,h=8,i=9,j=10,k=11,l=12,m=13,n=14,o=15,p=16,q=17,r=18;
float s = pow(1+2*pow(7+5*9, 2+3*8), 5);
a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r;
)";
    TEST_EQ(9 * 19, run(script, optimize, peephole, use_all_variable_registers));
    std::string script2 = R"(
()
float a=1,b=2,c=3,d=4,e=5,f=6,g=7,h=8,i=9,j=10,k=11,l=12,m=13,n=14,o=15,p=16,q=17,r=18;
int result[3];
result[2] = 0;
float s = pow(1+2*pow(7+5*9, 2+3*8+result[pow(1,0)]), 5);
a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r;
)";
    TEST_EQ(9 * 19, run(script2, optimize, peephole, use_all_variable_registers));

    std::string script3 = R"(
()
int result[3];
result[0] = 0;
result[1] = 0;
float s = pow(1+2*pow(7+5*9, 1+1*1+(++result[pow(1,0)])), 1);
s;
)";
    TEST_EQ(281217, run(script3, optimize, peephole, use_all_variable_registers));

    std::string script4 = R"(
()
float a=1,b=2,c=3,d=4,e=5,f=6,g=7,h=8,i=9,j=10,k=11,l=12,m=13,n=14,o=15,p=16,q=17,r=18;
float result[3];
result[0] = 0;
float s = pow(1+2*pow(7+5*9, 2+3*8+(++result[pow(1,0)])), 5);
a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r;
)";
    TEST_EQ(9 * 19, run(script4, optimize, peephole, use_all_variable_registers));


    std::string script5 = R"(
()
float a=1,b=2,c=3,d=4,e=5,f=6,g=7,h=8,i=9,j=10,k=11,l=12,m=13,n=14,o=15,p=16,q=17,r=18;
int result[3];
result[0] = 0;
result[1] = 0;
result[2] = 0;
float s = ++result[pow(1,0)+1];
a+b+c+d+e+f+g+h+i+j+k+l+m+n+o+p+q+r;
)";
    TEST_EQ(9 * 19, run(script5, optimize, peephole, use_all_variable_registers));

    }

  };

struct test_global_variables : public compile_fixture
  {
  void test(bool optimize = false, bool peephole = true, bool use_all_variable_registers = false)
    {
    TEST_EQ(3, run("()int $g1=3; float $g2 = 3.14; $g1;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(3.14, run("()$g2;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(6.1400000000000006, run("()$g1+$g2;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(3.14, run("()$g1=2;$g2;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(5.1400000000000006, run("()$g1+$g2;", optimize, peephole, use_all_variable_registers));
    TEST_EQ(10, run("()$g2=8;$g1+$g2;", optimize, peephole, use_all_variable_registers));
    }
  };

struct computation_order_test : public compile_fixture
  {
  void test(bool optimize, bool peephole, bool use_all_variable_registers)
    {
    std::string script = R"((int i)
i/127.0*4.0;
)";
    TEST_EQ(4.0, runi(script, 127, optimize, peephole, use_all_variable_registers));
    }
  };

COMPILER_END

void run_all_compile_tests()
  {
  using namespace COMPILER;

  for (int i = 0; i < 5; ++i)
    {
    bool optimize = true;
    bool peephole = true;
    bool use_all_variable_registers = false;
    switch (i)
      {
      case 0: optimize = true; peephole = true; use_all_variable_registers = false; break;
      case 1: optimize = true; peephole = false; use_all_variable_registers = false; break;
      case 2: optimize = false; peephole = true; use_all_variable_registers = false; break;
      case 3: optimize = false; peephole = false; use_all_variable_registers = false; break;
      case 4: optimize = false; peephole = false; use_all_variable_registers = true; break;
      }
    compiler_float().test(optimize, peephole, use_all_variable_registers);
    compiler_int().test(optimize, peephole, use_all_variable_registers);
    compiler_expr().test(optimize, peephole, use_all_variable_registers);
    compiler_relop().test(optimize, peephole, use_all_variable_registers);
    compiler_integer_var().test(optimize, peephole, use_all_variable_registers);
    compiler_float_var().test(optimize, peephole, use_all_variable_registers);
    compiler_assignment().test(optimize, peephole, use_all_variable_registers);
    compiler_array().test(optimize, peephole, use_all_variable_registers);
    comment_test().test(optimize, peephole, use_all_variable_registers);
    parameter_test().test(optimize, peephole, use_all_variable_registers);
    parameter_pointer_test().test(optimize, peephole, use_all_variable_registers);
    parameter_dereference_test().test(optimize, peephole, use_all_variable_registers);
    inc_dec_test().test(optimize, peephole, use_all_variable_registers);
    for_loop_test().test(optimize, peephole, use_all_variable_registers);
    optimize_tests().test(optimize, peephole, use_all_variable_registers);
    funccall_tests().test(optimize, peephole, use_all_variable_registers);
    rsp_offset_test().test(optimize, peephole, use_all_variable_registers);
    modulo_test().test(optimize, peephole, use_all_variable_registers);
    if_test().test(optimize, peephole, use_all_variable_registers);
    dead_code_test().test(optimize, peephole, use_all_variable_registers);
#if 0
    harmonic().test(optimize, peephole, use_all_variable_registers);
    fibonacci().test(optimize, peephole, use_all_variable_registers);
    hamming().test(optimize, peephole, use_all_variable_registers);
    quicksort().test(optimize, peephole, use_all_variable_registers);
    quicksortdouble().test(optimize, peephole, use_all_variable_registers);
    digitspi().test(optimize, peephole, use_all_variable_registers);
#endif
    test_many_variables().test(optimize, peephole, use_all_variable_registers);
    test_global_variables().test(optimize, peephole, use_all_variable_registers);
    array_assignment_test().test(optimize, peephole, use_all_variable_registers);
    external_function_test().test(optimize, peephole, use_all_variable_registers);
    array_address_test().test(optimize, peephole, use_all_variable_registers);
    computation_order_test().test(optimize, peephole, use_all_variable_registers);
    }
  test_strength_reduction().test();
  }
