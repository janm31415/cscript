///////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////

#include "vm/vm.h"

#include "cscript/compiler.h"
#include "cscript/optimize.h"
#include "cscript/parse.h"
#include "cscript/tokenize.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <fstream>

#include "test_assert.h"

COMPILER_BEGIN

struct compile_fixture
  {
  typedef float(__cdecl* fun_ptr)(...);
  compile_data data;
  compile_fixture()
    {
    }
  ~compile_fixture()
    {
    }
  VM::vmcode get_vmcode(const std::string& script, bool _optimize)
    {
    auto tokens = tokenize(script);
    auto prog = make_program(tokens);
    if (_optimize)
      optimize(prog);
    VM::vmcode code;
    compile(code, data, prog);
    return code;
    }

  double run(const std::string& script, bool _optimize = true)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    VM::registers reg;
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

  double runi(const std::string& script, int64_t i1, bool _optimize = true)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    VM::registers reg;
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

  double runii(const std::string& script, int64_t i1, int64_t i2, bool _optimize = true)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    VM::registers reg;
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

  double runiii(const std::string& script, int64_t i1, int64_t i2, int64_t i3, bool _optimize = true)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    VM::registers reg;
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

  double runiiii(const std::string& script, int64_t i1, int64_t i2, int64_t i3, int64_t i4, bool _optimize = true)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    VM::registers reg;
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

  float runiiiii(const std::string& script, int64_t i1, int64_t i2, int64_t i3, int64_t i4, int64_t i5, bool _optimize = true)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    VM::registers reg;
    reg.rcx = i1;
    reg.rdx = i2;
    reg.r8 = i3;
    reg.r9 = i4;
    uint64_t* mem = (uint64_t*)reg.rsp;
    *(mem - 16) = i5;
    reg.rsp -= 40;
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

  double runf(const std::string& script, double i1, bool _optimize = true)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    VM::registers reg;
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

  double runff(const std::string& script, double i1, double i2, bool _optimize = true)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    VM::registers reg;
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

  double runfff(const std::string& script, double i1, double i2, double i3, bool _optimize = true)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    VM::registers reg;
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

  double runffff(const std::string& script, double i1, double i2, double i3, double i4, bool _optimize = true)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    VM::registers reg;
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
  /*
  float runfffff(const std::string& script, float i1, float i2, float i3, float i4, float i5, bool _optimize = true)
    {
    float res = std::numeric_limits<float>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    typedef float(__cdecl *fun_ptr)(float, float, float, float, float);
    fun_ptr f = (fun_ptr)assemble(code);
    if (f)
      {
      res = f(i1, i2, i3, i4, i5);
      free_assembled_function(f);
      }
    return res;
    }

  float runififif(const std::string& script, int64_t i1, float f2, int64_t i3, float f4, int64_t i5, float f6, bool _optimize = true)
    {
    float res = std::numeric_limits<float>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    typedef float(__cdecl *fun_ptr)(int, float, int, float, int, float);
    fun_ptr f = (fun_ptr)assemble(code);
    if (f)
      {
      res = f(i1, f2, i3, f4, i5, f6);
      free_assembled_function(f);
      }
    return res;
    }
    */
  double runpi(const std::string& script, int64_t* i1, bool _optimize = true)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    VM::registers reg;
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

  double runpf(const std::string& script, double* f1, bool _optimize = true)
    {
    double res = std::numeric_limits<double>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    VM::registers reg;
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

  void test()
    {
    TEST_EQ(1.0, run("()1.f;"));
    TEST_EQ(3.14, run("()3.14;"));
    TEST_EQ(-9.0, run("()3.0*(2/0.5-7);"));
    TEST_EQ(9.0, run("()-3.0*(2/0.5-7);"));
    }
  };

struct compiler_int : public compile_fixture
  {
  void test()
    {
    TEST_EQ(1.0, run("()1;"));
    TEST_EQ(110.0, run("()110;"));
    TEST_EQ(135.0, run("()110+25;"));
    TEST_EQ(-135.0, run("()-(110+25);"));
    }
  };

struct compiler_expr : public compile_fixture
  {
  void test()
    {
    TEST_EQ(5.5, run("()3.0+2.5;", false));
    TEST_EQ(-9.0, run("()3.0*(2/0.5-7);", false));
    TEST_EQ(9.0, run("()-3.0*(2/0.5-7);", false));
    TEST_EQ(55.0, run("()1.0 + 2.0 + 3.0 + 4.0 + 5.0 + 6.0 + 7.0 + 8.0 + 9.0 + 10.0;", false));
    TEST_ASSERT(55.0 == run("()1.0 + (2.0 + (3.0 + (4.0 + (5.0 + (6.0 + (7.0 + (8.0 + (9.0 + 10.0))))))));", false));
    TEST_ASSERT(55.0 == run("()1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10;", false));
    TEST_ASSERT(55.0 == run("()1 + (2 + (3 + (4 + (5 + (6 + (7 + (8 + (9 + 10))))))));", false));
    TEST_EQ(-53.0, run("()1.0 - 2.0 - 3.0 - 4.0 - 5.0 - 6.0 - 7.0 - 8.0 - 9.0 - 10.0;", false));
    TEST_EQ(-5.0, run("()1.0 - (2.0 - (3.0 - (4.0 - (5.0 - (6.0 - (7.0 - (8.0 - (9.0 - 10.0))))))));", false));
    TEST_EQ(-53.0, run("()1 - 2 - 3 - 4 - 5 - 6 - 7 - 8 - 9 - 10;", false));
    TEST_EQ(-5.0, run("()1 - (2 - (3 - (4 - (5 - (6 - (7 - (8 - (9 - 10))))))));", false));
    TEST_EQ(3628800.0, run("()1.0 * 2.0 * 3.0 * 4.0 * 5.0 * 6.0 * 7.0 * 8.0 * 9.0 * 10.0;", false));
    TEST_EQ(3628800.0, run("()1.0 * (2.0 * (3.0 * (4.0 * (5.0 * (6.0 * (7.0 * (8.0 * (9.0 * 10.0))))))));", false));
    TEST_EQ(3628800.0, run("()1 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9 * 10;", false));
    TEST_EQ(3628800.0, run("()1 * (2 * (3 * (4 * (5 * (6 * (7 * (8 * (9 * 10))))))));", false));
    TEST_EQ_CLOSE(2.75573200e-07, run("()1.0 / 2.0 / 3.0 / 4.0 / 5.0 / 6.0 / 7.0 / 8.0 / 9.0 / 10.0;", false), 1e-7);
    TEST_EQ(1, run("()5 / 3;", false));
    TEST_EQ(2, run("()6 / 3;", false));
    TEST_EQ(2, run("()8 / 2 / 2;", false));
    TEST_EQ(32, run("()1024 / 2 / 2 / 2 / 2 / 2;", false));
    TEST_EQ(32, run("() ((((1024 / 2) / 2) / 2) / 2) / 2;", false));
    TEST_EQ(8, run("()1024 / (512 / (256 / (128 / (64 / 32))));", false));
    }
  };

struct compiler_relop : public compile_fixture
  {
  void test()
    {
    TEST_EQ(0, run("()5.0 < 3.0;"));
    TEST_EQ(1, run("()3.0 < 5.0;"));
    TEST_EQ(0, run("()5.0 < 3.0;", false));
    TEST_EQ(1, run("()3.0 < 5.0;", false));

    TEST_EQ(0, run("()5 < 3;"));
    TEST_EQ(1, run("()3 < 5;"));
    TEST_EQ(0, run("()5 < 3;", false));
    TEST_EQ(1, run("()3 < 5;", false));

    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(5.0 < 3.0)))));", false));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(3.0 < 5.0)))));", false));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 < 3.0))))));", false));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 < 5.0))))));", false));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 < 3.0)))))));", false));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 < 5.0)))))));", false));

    TEST_EQ(1, run("()(1 * (3 < 5));", false));
    TEST_EQ(1, run("()(1 * (1 * (3 < 5)));", false));
    TEST_EQ(0, run("()(1 * (5 < 3));", false));
    TEST_EQ(0, run("()(1 * (1 * (5 < 3)));", false));


    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(5.0 > 3.0)))));", false));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(3.0 > 5.0)))));", false));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 > 3.0))))));", false));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 > 5.0))))));", false));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 > 3.0)))))));", false));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 > 5.0)))))));", false));

    TEST_EQ(1, run("()5 > 3;"));
    TEST_EQ(0, run("()3 > 5;"));
    TEST_EQ(1, run("()5 > 3;", false));
    TEST_EQ(0, run("()3 > 5;", false));
    TEST_EQ(0, run("()(1 * (3 > 5));", false));
    TEST_EQ(0, run("()(1 * (1 * (3 > 5)));", false));
    TEST_EQ(1, run("()(1 * (5 > 3));", false));
    TEST_EQ(1, run("()(1 * (1 * (5 > 3)));", false));

    TEST_EQ(0, run("()5.0 <= 3.0;", false));
    TEST_EQ(1, run("()3.0 <= 5.0;", false));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(5.0 <= 3.0)))));", false));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(3.0 <= 5.0)))));", false));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 <= 3.0))))));", false));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 <= 5.0))))));", false));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 <= 3.0)))))));", false));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 <= 5.0)))))));", false));

    TEST_EQ(0, run("()5 <= 3;"));
    TEST_EQ(1, run("()3 <= 5;"));
    TEST_EQ(0, run("()5 <= 3;", false));
    TEST_EQ(1, run("()3 <= 5;", false));
    TEST_EQ(1, run("()(1 * (3 <= 5));", false));
    TEST_EQ(1, run("()(1 * (1 * (3 <= 5)));", false));
    TEST_EQ(0, run("()(1 * (5 <= 3));", false));
    TEST_EQ(0, run("()(1 * (1 * (5 <= 3)));", false));

    TEST_EQ(1, run("()5 >= 3;"));
    TEST_EQ(0, run("()3 >= 5;"));
    TEST_EQ(1, run("()5 >= 3;", false));
    TEST_EQ(0, run("()3 >= 5;", false));
    TEST_EQ(0, run("()(1 * (3 >= 5));", false));
    TEST_EQ(0, run("()(1 * (1 * (3 >= 5)));", false));
    TEST_EQ(1, run("()(1 * (5 >= 3));", false));
    TEST_EQ(1, run("()(1 * (1 * (5 >= 3)));", false));

    TEST_EQ(1, run("()5.0 >= 3.0;", false));
    TEST_EQ(0, run("()3.0 >= 5.0;", false));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(5.0 >= 3.0)))));", false));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(3.0 >= 5.0)))));", false));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 >= 3.0))))));", false));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 >= 5.0))))));", false));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 >= 3.0)))))));", false));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 >= 5.0)))))));", false));

    TEST_EQ(0, run("()5 == 3;", false));
    TEST_EQ(1, run("()3 == 3;", false));
    TEST_EQ(0, run("()(1 * (3 == 5));", false));
    TEST_EQ(0, run("()(1 * (1 * (3 == 5)));", false));
    TEST_EQ(1, run("()(1 * (3 == 3));", false));
    TEST_EQ(1, run("()(1 * (1 * (3 == 3)));", false));
    TEST_EQ(0, run("()5.0 == 3.0;", false));
    TEST_EQ(1, run("()3.0 == 3.0;", false));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(5.0 == 3.0)))));", false));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(3.0 == 3.0)))));", false));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 == 3.0))))));", false));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 == 3.0))))));", false));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 == 3.0)))))));", false));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 == 3.0)))))));", false));

    TEST_EQ(1, run("()5 != 3;", false));
    TEST_EQ(0, run("()3 != 3;", false));
    TEST_EQ(1, run("()(1 * (3 != 5));", false));
    TEST_EQ(1, run("()(1 * (1 * (3 != 5)));", false));
    TEST_EQ(0, run("()(1 * (3 != 3));", false));
    TEST_EQ(0, run("()(1 * (1 * (3 != 3)));", false));

    TEST_EQ(1, run("()5.0 != 3.0;", false));
    TEST_EQ(0, run("()3.0 != 3.0;", false));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(5.0 != 3.0)))));", false));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(3.0 != 3.0)))));", false));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 != 3.0))))));", false));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 != 3.0))))));", false));
    TEST_EQ(1, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 != 3.0)))))));", false));
    TEST_EQ(0, run("()(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 != 3.0)))))));", false));
    }
  };

struct compiler_integer_var : public compile_fixture
  {
  void test()
    {
    TEST_EQ(3, run("() int i = 3; i;", false));
    TEST_EQ(1512, run("() int i = 3*7*8*9; i;", false));
    TEST_EQ(3, run("() int i = 3.0; i;", false));
    TEST_EQ(1512, run("() int i = 3.0*7.0*8.0*9.0; i;", false));
    }
  };

struct compiler_float_var : public compile_fixture
  {
  void test()
    {
    TEST_EQ(3, run("() float f = 3.0; f;", false));
    TEST_EQ(1512, run("() float f = 3.0*7.0*8.0*9.0; f;", false));
    TEST_EQ(3, run("() float f = 3; f;", false));
    TEST_EQ(1512, run("() float f = 3*7*8*9; f;", false));
    }
  };

struct compiler_assignment : public compile_fixture
  {
  void test()
    {
    TEST_EQ(4.1415000000000006, run("() float f; int i; f = 3.1415; i = 1; f+i;", false));
    TEST_EQ(4.1415000000000006, run("() float f; int i; f = 3.1415; i = 1; f+=i;f;", false));
    TEST_EQ(4.0, run("() float f; int i; f = 3.1415; i = 1; i+=f;i;", false));

    TEST_EQ(2.1415, run("() float f; int i; f = 3.1415; i = 1; f-=i;f;", false));
    TEST_EQ(-2.0, run("() float f; int i; f = 3.1415; i = 1; i-=f;i;", false));

    TEST_EQ(6.2830, run("() float f; int i; f = 3.1415; i = 2; f*=i;f;", false));
    TEST_EQ(6., run("() float f; int i; f = 3.1415; i = 2; i*=f;i;", false));

    TEST_EQ(1.5707500000000001, run("() float f; int i; f = 3.1415; i = 2; f/=i;f;", false));
    TEST_EQ(3.0, run("() float f; int i; f = 3.1415; i = 9; i/=f;i;", false));

    TEST_EQ(4.0, run("() int i = 9; i/=(1+1);i;", false));
    }
  };

struct compiler_array : public compile_fixture
  {
  void test()
    {
    TEST_EQ(6.0, run("() float f[3];\nf[0] = 1;\nf[1] = 2;\nf[2] = 3;\nf[0]*f[1]*f[2];", false));
    TEST_EQ(6.0, run("() int f[3];\nf[0] = 1;\nf[1] = 2;\nf[2] = 3;\nf[0]*f[1]*f[2];", false));

    TEST_EQ(1.0 + 3.14, run("() float f[10];\nf[8] = 1;\nf[8] += 3.14;f[8];", false));
    TEST_EQ(4.0, run("() int f[10];\nf[8] = 1;\nf[8] += 3;f[8];", false));

    TEST_EQ(1.0 - 3.14, run("() float f[10];\nf[8] = 1;\nf[8] -= 3.14;f[8];", false));
    TEST_EQ(-2.0, run("() int f[10];\nf[8] = 1;\nf[8] -= 3;f[8];", false));

    TEST_EQ(6.28, run("() float f[10];\nf[8] = 2;\nf[8] *= 3.14;f[8];", false));
    TEST_EQ(6.0, run("() int f[10];\nf[8] = 2;\nf[8] *= 3;f[8];", false));

    TEST_EQ(15.8 / 2.15, run("() float f[10];\nf[8] = 15.8;\nf[8] /= 2.15;f[8];", false));
    TEST_EQ(3.0, run("() int f[10];\nf[8] = 10;\nf[8] /= 3;f[8];", false));
    }
  };

struct comment_test : public compile_fixture
  {
  void test()
    {
    std::string script =
      R"( ()
int i[3]; // make an array of 3 integers
i[1] = 8; // assign 8 to index 1 position
i[1] /= 2; // divide index 1 position by 2
i[1]; // return the result in index 1
    )";

    TEST_EQ(4, run(script));

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

    TEST_EQ(8, run(script));
    }
  };

struct parameter_test : public compile_fixture
  {
  void test()
    {
    TEST_EQ(4.f, runi("(int i) i + 1;", 3, false));
    TEST_EQ(10.f, runii("(int i, int j) i + j;", 3, 7, false));
    TEST_EQ(15.f, runiii("(int i, int j, int k) i + j + k;", 3, 7, 5, false));
    TEST_EQ(35.f, runiiii("(int i, int j, int k, int l) i + j + k + l;", 3, 7, 5, 20, false));
    //TEST_EQ(135.f, runiiiii("(int i, int j, int k, int l, int m) i + j + k + l + m;", 3, 7, 5, 20, 100, false));

    TEST_EQ(1.2 + 1.5, runf("(float f) f + 1.5;", 1.2, false));
    TEST_EQ(1.2 + 1.5, runff("(float f, float g) f + g;", 1.2, 1.5, false));
    TEST_EQ(1.2 + 1.5 + 7.9, runfff("(float f, float g, float h) f + g + h;", 1.2, 1.5, 7.9, false));
    TEST_EQ(1.2 + 1.5 + 7.9 + 10.4, runffff("(float f, float g, float h, float i) f + g + h + i;", 1.2, 1.5, 7.9, 10.4, false));
    //TEST_EQ(1.2f + 1.5f + 7.9f + 10.4f + 19.1f, runfffff("(float f, float g, float h, float i, float j) f + g + h + i + j;", 1.2f, 1.5f, 7.9f, 10.4f, 19.1f, false));
    //
    //
    //TEST_EQ(1.f + 1.5f + 7.f + 10.4f + 19.f + 3.14f, runififif("(int i1, float f2, int i3, float f4, int i5, float f6) i1+f2+i3+f4+i5+f6;", 1, 1.5f, 7, 10.4f, 19, 3.14f, false));
    }
  };

struct parameter_pointer_test : public compile_fixture
  {
  void test()
    {
    int64_t i[3];
    double f[3];
    runpi("(int* i) i[0] = 0; i[1] = 1; i[2] = 2;", &i[0], false);
    TEST_EQ(0, i[0]);
    TEST_EQ(1, i[1]);
    TEST_EQ(2, i[2]);
    runpf("(float* f) f[0] = 0.5; f[1] = 1.5; f[2] = 2.5;", &f[0], false);
    TEST_EQ(0.5, f[0]);
    TEST_EQ(1.5, f[1]);
    TEST_EQ(2.5, f[2]);
    runpi("(int* i) i[0] += 7; i[1] += 4; i[2] += 2;", &i[0], false);
    TEST_EQ(7, i[0]);
    TEST_EQ(5, i[1]);
    TEST_EQ(4, i[2]);
    runpf("(float* f) f[0] += 0.1; f[1] += 0.2; f[2] += 0.3;", &f[0], false);
    TEST_EQ(0.5 + 0.1, f[0]);
    TEST_EQ(1.5 + 0.2, f[1]);
    TEST_EQ(2.5 + 0.3, f[2]);
    runpi("(int* i) i[0] -= 10; i[1] -= 4; i[2] -= 21;", &i[0], false);
    TEST_EQ(-3, i[0]);
    TEST_EQ(1, i[1]);
    TEST_EQ(-17, i[2]);
    runpf("(float* f) f[0] -= 0.1; f[1] -= 0.2; f[2] -= 0.3;", &f[0], false);
    TEST_EQ(0.5, f[0]);
    TEST_EQ(1.5, f[1]);
    TEST_EQ(2.5, f[2]);
    runpi("(int* i) i[0] *= 2;", &i[0], false);
    TEST_EQ(-6, i[0]);
    TEST_EQ(1, i[1]);
    TEST_EQ(-17, i[2]);
    runpf("(float* f) f[1] *= 3.14f;", &f[0], false);
    TEST_EQ(0.5, f[0]);
    TEST_EQ(1.5 * 3.14, f[1]);
    TEST_EQ(2.5, f[2]);
    runpi("(int* i) i[0] /= 2;", &i[0], false);
    TEST_EQ(-3, i[0]);
    TEST_EQ(1, i[1]);
    TEST_EQ(-17, i[2]);
    runpf("(float* f) f[1] /= 3.14f;", &f[0], false);
    TEST_EQ(0.5, f[0]);
    TEST_EQ(1.5, f[1]);
    TEST_EQ(2.5, f[2]);
    }
  };

struct parameter_dereference_test : public compile_fixture
  {
  void test()
    {
    double f = 3.14;
    int64_t i = 5;
    TEST_EQ(3.14, runpf("(float* f) float g = *f; g;", &f, false));
    TEST_EQ(5, runpi("(int* i) int g = *i; g;", &i, false));
    TEST_EQ(10.14, runpf("(float* f) float g = (1 + (1 + (1 + (1 + (1 + (1 + ( 1 + *f))))))); g;", &f, false));
    TEST_EQ(12, runpi("(int* i) int g = (1 + (1 + (1 + (1 + (1 + (1 + ( 1 + *i))))))); g;", &i, false));

    TEST_EQ(3.14, runpf("(float* f) float g = f[0]; g;", &f, false));
    TEST_EQ(5, runpi("(int* i) int g = i[0]; g;", &i, false));
    TEST_EQ(10.14, runpf("(float* f) float g = (1 + (1 + (1 + (1 + (1 + (1 + ( 1 + f[0]))))))); g;", &f, false));
    TEST_EQ(12, runpi("(int* i) int g = (1 + (1 + (1 + (1 + (1 + (1 + ( 1 + i[0]))))))); g;", &i, false));

    TEST_EQ(3.14 * 3.14, runpf("(float* f) *f * *f;", &f, false));
    TEST_EQ(25.0, runpi("(int* i) *i * *i;", &i, false));

    runpf("(float* f) *f += 1.2;", &f, false);
    TEST_EQ(3.14 + 1.2, f);
    runpf("(float* f) *f = 1.2;", &f, false);
    TEST_EQ(1.2, f);
    runpf("(float* f) *f -= 1.4;", &f, false);
    TEST_EQ(1.2 - 1.4, f);
    runpf("(float* f) *f *= 2.8;", &f, false);
    TEST_EQ((1.2 - 1.4) * 2.8, f);
    runpf("(float* f) *f /= 2.5;", &f, false);
    TEST_EQ((1.2 - 1.4) * 2.8 / 2.5, f);

    runpi("(int* i) *i += 2;", &i, false);
    TEST_EQ(7, i);
    runpi("(int* i) *i = 2;", &i, false);
    TEST_EQ(2, i);
    runpi("(int* i) *i -= 9;", &i, false);
    TEST_EQ(-7, i);
    runpi("(int* i) *i *= 9;", &i, false);
    TEST_EQ(-63, i);
    runpi("(int* i) *i /= 6;", &i, false);
    TEST_EQ(-10, i);
    }
  };

struct inc_dec_test : public compile_fixture
  {
  void test()
    {
    TEST_EQ(4, run("() int i = 3; ++i;", false));
    TEST_EQ(2, run("() int i = 3; --i;", false));
    TEST_EQ(4, run("() int i = 3; ++i; i;", false));
    TEST_EQ(2, run("() int i = 3; --i; i;", false));

    TEST_EQ(4, run("() float i = 3; ++i;", false));
    TEST_EQ(2, run("() float i = 3; --i;", false));
    TEST_EQ(4, run("() float i = 3; ++i; i;", false));
    TEST_EQ(2, run("() float i = 3; --i; i;", false));

    TEST_EQ(4, run("() int i[4]; i[2] = 3; ++i[2];", false));
    TEST_EQ(4, run("() int i[4]; i[2] = 3; ++i[2]; i[2];", false));
    TEST_EQ(2, run("() int i[4]; i[2] = 3; --i[2];", false));
    TEST_EQ(2, run("() int i[4]; i[2] = 3; --i[2]; i[2];", false));

    TEST_EQ(4, run("() float i[4]; i[2] = 3; ++i[2];", false));
    TEST_EQ(4, run("() float i[4]; i[2] = 3; ++i[2]; i[2];", false));
    TEST_EQ(2, run("() float i[4]; i[2] = 3; --i[2];", false));
    TEST_EQ(2, run("() float i[4]; i[2] = 3; --i[2]; i[2];", false));

    int64_t i[3];
    runpi("(int* i) i[0] = 0; i[1] = 3; i[2] = 6; ++i[1];", &i[0], false);
    TEST_EQ(0, i[0]);
    TEST_EQ(4, i[1]);
    TEST_EQ(6, i[2]);

    double f[3];
    runpf("(float* f) f[0] = 0; f[1] = 3; f[2] = 6; ++f[1];", &f[0], false);
    TEST_EQ(0.f, f[0]);
    TEST_EQ(4.f, f[1]);
    TEST_EQ(6.f, f[2]);

    int64_t i1 = 7;
    runpi("(int* i) ++*i;", &i1, false);
    TEST_EQ(8, i1);
    runpi("(int* i) --*i;", &i1, false);
    TEST_EQ(7, i1);

    double f1 = 7.5;
    runpf("(float* i) ++*i;", &f1, false);
    TEST_EQ(8.5f, f1);
    runpf("(float* i) --*i;", &f1, false);
    TEST_EQ(7.5f, f1);
    }
  };

struct for_loop_test : public compile_fixture
  {
  void test()
    {
    TEST_EQ(1225.f, run("() float f = 0; for (int i = 0; i < 50; ++i) { f += i; } f;", false));
    }
  };

struct harmonic : public compile_fixture
  {
  void test()
    {
    auto tic = std::chrono::high_resolution_clock::now();
    TEST_EQ_CLOSE(14.3927, run("() float sum = 0.0; for (int i = 1; i<1000000; ++i) { sum += 1.0/i; } sum;", false), 1e-4);    
    auto toc = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(toc - tic).count();
    printf("Harmonic timing: %dms\n", ms);
    }
  };

struct optimize_tests : public compile_fixture
  {
  void test()
    {
    TEST_EQ(0, run("() int i = 0;", false));
    TEST_EQ(0, run("() float f = 0.0;", false));
    TEST_EQ(0, run("() float f = 0;", false));
    }
  };

struct funccall_tests : public compile_fixture
  {
  void test()
    {
    TEST_EQ(std::sqrt(2.0), run("() sqrt(2.f);", false));
    TEST_EQ(std::sqrt(2.0), run("() sqrt(2.f);", true));

    TEST_EQ(std::sin(0.5), run("() sin(0.5f);", false));
    TEST_EQ(std::sin(0.5), run("() sin(0.5f);", true));

    TEST_EQ(std::cos(0.5), run("() cos(0.5f);", false));
    TEST_EQ(std::cos(0.5), run("() cos(0.5f);", true));

    //TEST_EQ(std::exp(0.5), run("() exp(0.5f);", false));
    //TEST_EQ(std::exp(0.5), run("() exp(0.5f);", true));

    TEST_EQ(std::log(0.5), run("() log(0.5f);", false));
    TEST_EQ(std::log(0.5), run("() log(0.5f);", true));

    TEST_EQ(std::log2(0.5), run("() log2(0.5f);", false));
    TEST_EQ(std::log2(0.5), run("() log2(0.5f);", true));

    TEST_EQ(std::fabs(-0.5), run("() fabs(0.5f);", false));
    TEST_EQ(std::fabs(-0.5), run("() fabs(0.5f);", true));
    }
  };

struct rsp_offset_test : public compile_fixture
  {
  void test()
    {
    TEST_EQ_CLOSE(0.0, runf("(float x) float y = 1.0 - x; sin(y*y*3.141592653589793238462643383*4.0);", 1.f), 1e-5);
    TEST_EQ_CLOSE(0.0, runf("(float x) float y = 1.0 - x; sin(y*y*3.141592653589793238462643383*4.0);", 0.f), 1e-5);
    }
  };

COMPILER_END

void run_all_compile_tests()
  {
  using namespace COMPILER;

  compiler_float().test();
  compiler_int().test();
  compiler_expr().test();
  compiler_relop().test();
  compiler_integer_var().test();
  compiler_float_var().test();
  compiler_assignment().test();
  compiler_array().test();
  comment_test().test();
  parameter_test().test();
  parameter_pointer_test().test();
  parameter_dereference_test().test();
  inc_dec_test().test();
  for_loop_test().test();
  optimize_tests().test();
  funccall_tests().test();
  rsp_offset_test().test();
  //harmonic().test();
  }