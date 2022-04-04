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

  float run(const std::string& script, bool _optimize = true)
    {
    float res = std::numeric_limits<float>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    uint64_t size;
    uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
    VM::registers reg;
    try {
      VM::run_bytecode(f, size, reg);
      }
    catch (std::logic_error e)
      {
      std::cout << e.what() << "\n";
      }
    VM::free_bytecode(f, size);

    return res;
    }
  /*
  float runi(const std::string& script, int i1, bool _optimize = true)
    {
    float res = std::numeric_limits<float>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    fun_ptr f = (fun_ptr)assemble(code);
    if (f)
      {
      res = f(i1);
      free_assembled_function(f);
      }
    return res;
    }

  float runii(const std::string& script, int i1, int i2, bool _optimize = true)
    {
    float res = std::numeric_limits<float>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    fun_ptr f = (fun_ptr)assemble(code);
    if (f)
      {
      res = f(i1, i2);
      free_assembled_function(f);
      }
    return res;
    }

  float runiii(const std::string& script, int i1, int i2, int i3, bool _optimize = true)
    {
    float res = std::numeric_limits<float>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    fun_ptr f = (fun_ptr)assemble(code);
    if (f)
      {
      res = f(i1, i2, i3);
      free_assembled_function(f);
      }
    return res;
    }

  float runiiii(const std::string& script, int i1, int i2, int i3, int i4, bool _optimize = true)
    {
    float res = std::numeric_limits<float>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    fun_ptr f = (fun_ptr)assemble(code);
    if (f)
      {
      res = f(i1, i2, i3, i4);
      free_assembled_function(f);
      }
    return res;
    }

  float runiiiii(const std::string& script, int i1, int i2, int i3, int i4, int i5, bool _optimize = true)
    {
    float res = std::numeric_limits<float>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    fun_ptr f = (fun_ptr)assemble(code);
    if (f)
      {
      res = f(i1, i2, i3, i4, i5);
      free_assembled_function(f);
      }
    return res;
    }

  float runf(const std::string& script, float i1, bool _optimize = true)
    {
    float res = std::numeric_limits<float>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    typedef float(__cdecl *fun_ptr)(float);
    fun_ptr f = (fun_ptr)assemble(code);
    if (f)
      {
      res = f(i1);
      free_assembled_function(f);
      }
    return res;
    }

  float runff(const std::string& script, float i1, float i2, bool _optimize = true)
    {
    float res = std::numeric_limits<float>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    typedef float(__cdecl *fun_ptr)(float, float);
    fun_ptr f = (fun_ptr)assemble(code);
    if (f)
      {
      res = f(i1, i2);
      free_assembled_function(f);
      }
    return res;
    }

  float runfff(const std::string& script, float i1, float i2, float i3, bool _optimize = true)
    {
    float res = std::numeric_limits<float>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    typedef float(__cdecl *fun_ptr)(float, float, float);
    fun_ptr f = (fun_ptr)assemble(code);
    if (f)
      {
      res = f(i1, i2, i3);
      free_assembled_function(f);
      }
    return res;
    }

  float runffff(const std::string& script, float i1, float i2, float i3, float i4, bool _optimize = true)
    {
    float res = std::numeric_limits<float>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    typedef float(__cdecl *fun_ptr)(float, float, float, float);
    fun_ptr f = (fun_ptr)assemble(code);
    if (f)
      {
      res = f(i1, i2, i3, i4);
      free_assembled_function(f);
      }
    return res;
    }

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

  float runififif(const std::string& script, int i1, float f2, int i3, float f4, int i5, float f6, bool _optimize = true)
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

  float runpi(const std::string& script, int* i1, bool _optimize = true)
    {
    float res = std::numeric_limits<float>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    typedef float(__cdecl *fun_ptr)(int*);
    fun_ptr f = (fun_ptr)assemble(code);
    if (f)
      {
      res = f(i1);
      free_assembled_function(f);
      }
    return res;
    }

  float runpf(const std::string& script, float* f1, bool _optimize = true)
    {
    float res = std::numeric_limits<float>::quiet_NaN();
    auto code = get_vmcode(script, _optimize);
    typedef float(__cdecl *fun_ptr)(float*);
    fun_ptr f = (fun_ptr)assemble(code);
    if (f)
      {
      res = f(f1);
      free_assembled_function(f);
      }
    return res;
    }
    */
  };

struct compiler_float : public compile_fixture
  {

  void test()
    {
    TEST_EQ(1.f, run("()1.f;"));
    TEST_EQ(3.14000010f, run("()3.14;"));
    TEST_EQ(-9.f, run("()3.0*(2/0.5-7);"));
    TEST_EQ(9.f, run("()-3.0*(2/0.5-7);"));
    }
  };

/*
TEST_FIXTURE(compile_fixture, compiler_int)
  {
  TEST_EQ(1.f, run("()1;"));
  TEST_EQ(110.f, run("()110;"));
  TEST_EQ(135.f, run("()110+25;"));
  TEST_EQ(-135.f, run("()-(110+25);"));
  }

TEST_FIXTURE(compile_fixture, compiler_expr)
  {
  TEST_EQ(5.5f, run("()3.0+2.5;", false));
  TEST_EQ(-9.f, run("()3.0*(2/0.5-7);", false));
  TEST_EQ(9.f, run("()-3.0*(2/0.5-7);", false));
  TEST_EQ(55.f, run("()1.0 + 2.0 + 3.0 + 4.0 + 5.0 + 6.0 + 7.0 + 8.0 + 9.0 + 10.0;", false));
  CHECK(55.f == run("()1.0 + (2.0 + (3.0 + (4.0 + (5.0 + (6.0 + (7.0 + (8.0 + (9.0 + 10.0))))))));", false));
  CHECK(55.f == run("()1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10;", false));
  CHECK(55.f == run("()1 + (2 + (3 + (4 + (5 + (6 + (7 + (8 + (9 + 10))))))));", false));
  TEST_EQ(-53.f, run("()1.0 - 2.0 - 3.0 - 4.0 - 5.0 - 6.0 - 7.0 - 8.0 - 9.0 - 10.0;", false));
  TEST_EQ(-5.f, run("()1.0 - (2.0 - (3.0 - (4.0 - (5.0 - (6.0 - (7.0 - (8.0 - (9.0 - 10.0))))))));", false));
  TEST_EQ(-53.f, run("()1 - 2 - 3 - 4 - 5 - 6 - 7 - 8 - 9 - 10;", false));
  TEST_EQ(-5.f, run("()1 - (2 - (3 - (4 - (5 - (6 - (7 - (8 - (9 - 10))))))));", false));
  TEST_EQ(3628800.f, run("()1.0 * 2.0 * 3.0 * 4.0 * 5.0 * 6.0 * 7.0 * 8.0 * 9.0 * 10.0;", false));
  CHECK(3628800.f == run("()1.0 * (2.0 * (3.0 * (4.0 * (5.0 * (6.0 * (7.0 * (8.0 * (9.0 * 10.0))))))));", false));
  CHECK(3628800.f == run("()1 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9 * 10;", false));
  CHECK(3628800.f == run("()1 * (2 * (3 * (4 * (5 * (6 * (7 * (8 * (9 * 10))))))));", false));

  TEST_EQ_CLOSE(2.75573200e-07, run("()1.0 / 2.0 / 3.0 / 4.0 / 5.0 / 6.0 / 7.0 / 8.0 / 9.0 / 10.0;", false), 1e-7);
  TEST_EQ(1, run("()5 / 3;", false));
  TEST_EQ(2, run("()6 / 3;", false));
  TEST_EQ(2, run("()8 / 2 / 2;", false));
  TEST_EQ(32, run("()1024 / 2 / 2 / 2 / 2 / 2;", false));
  TEST_EQ(32, run("() ((((1024 / 2) / 2) / 2) / 2) / 2;", false));
  TEST_EQ(8, run("()1024 / (512 / (256 / (128 / (64 / 32))));", false));
  }

TEST_FIXTURE(compile_fixture, compiler_relop)
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

TEST_FIXTURE(compile_fixture, compiler_integer_var)
  {
  TEST_EQ(3, run("() int i = 3; i;", false));
  TEST_EQ(1512, run("() int i = 3*7*8*9; i;", false));
  TEST_EQ(3, run("() int i = 3.0; i;", false));
  TEST_EQ(1512, run("() int i = 3.0*7.0*8.0*9.0; i;", false));
  }

TEST_FIXTURE(compile_fixture, compiler_float_var)
  {
  TEST_EQ(3, run("() float f = 3.0; f;", false));
  TEST_EQ(1512, run("() float f = 3.0*7.0*8.0*9.0; f;", false));
  TEST_EQ(3, run("() float f = 3; f;", false));
  TEST_EQ(1512, run("() float f = 3*7*8*9; f;", false));
  }

TEST_FIXTURE(compile_fixture, compiler_assignment)
  {
  TEST_EQ(4.1415f, run("() float f; int i; f = 3.1415; i = 1; f+i;", false));
  TEST_EQ(4.1415f, run("() float f; int i; f = 3.1415; i = 1; f+=i;f;", false));
  TEST_EQ(4.f, run("() float f; int i; f = 3.1415; i = 1; i+=f;i;", false));

  TEST_EQ(2.1415f, run("() float f; int i; f = 3.1415; i = 1; f-=i;f;", false));
  TEST_EQ(-2.f, run("() float f; int i; f = 3.1415; i = 1; i-=f;i;", false));

  TEST_EQ(6.2830f, run("() float f; int i; f = 3.1415; i = 2; f*=i;f;", false));
  TEST_EQ(6.f, run("() float f; int i; f = 3.1415; i = 2; i*=f;i;", false));

  TEST_EQ(1.57075f, run("() float f; int i; f = 3.1415; i = 2; f/=i;f;", false));
  TEST_EQ(3.f, run("() float f; int i; f = 3.1415; i = 9; i/=f;i;", false));

  TEST_EQ(4.f, run("() int i = 9; i/=(1+1);i;", false));
  }

TEST_FIXTURE(compile_fixture, compiler_array)
  {
  TEST_EQ(6.f, run("() float f[3];\nf[0] = 1;\nf[1] = 2;\nf[2] = 3;\nf[0]*f[1]*f[2];", false));
  TEST_EQ(6.f, run("() int f[3];\nf[0] = 1;\nf[1] = 2;\nf[2] = 3;\nf[0]*f[1]*f[2];", false));

  TEST_EQ(1.f+3.14f, run("() float f[10];\nf[8] = 1;\nf[8] += 3.14;f[8];", false));
  TEST_EQ(4, run("() int f[10];\nf[8] = 1;\nf[8] += 3;f[8];", false));

  TEST_EQ(1.f - 3.14f, run("() float f[10];\nf[8] = 1;\nf[8] -= 3.14;f[8];", false));
  TEST_EQ(-2, run("() int f[10];\nf[8] = 1;\nf[8] -= 3;f[8];", false));

  TEST_EQ(6.28f, run("() float f[10];\nf[8] = 2;\nf[8] *= 3.14;f[8];", false));
  TEST_EQ(6, run("() int f[10];\nf[8] = 2;\nf[8] *= 3;f[8];", false));

  TEST_EQ(15.8f/2.15f, run("() float f[10];\nf[8] = 15.8;\nf[8] /= 2.15;f[8];", false));
  TEST_EQ(3, run("() int f[10];\nf[8] = 10;\nf[8] /= 3;f[8];", false));
  }

TEST_FIXTURE(compile_fixture, comment_test)
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
*//*
    )";

  TEST_EQ(8, run(script));
  }

TEST_FIXTURE(compile_fixture, parameter_test)
  {
  TEST_EQ(4.f, runi("(int i) i + 1;", 3, false));
  TEST_EQ(10.f, runii("(int i, int j) i + j;", 3, 7, false));
  TEST_EQ(15.f, runiii("(int i, int j, int k) i + j + k;", 3, 7, 5, false));
  TEST_EQ(35.f, runiiii("(int i, int j, int k, int l) i + j + k + l;", 3, 7, 5, 20, false));
  TEST_EQ(135.f, runiiiii("(int i, int j, int k, int l, int m) i + j + k + l + m;", 3, 7, 5, 20, 100, false));

  TEST_EQ(1.2f + 1.5f, runf("(float f) f + 1.5;", 1.2f, false));
  TEST_EQ(1.2f + 1.5f, runff("(float f, float g) f + g;", 1.2f, 1.5f, false));
  TEST_EQ(1.2f + 1.5f + 7.9f, runfff("(float f, float g, float h) f + g + h;", 1.2f, 1.5f, 7.9f, false));
  TEST_EQ(1.2f + 1.5f + 7.9f + 10.4f, runffff("(float f, float g, float h, float i) f + g + h + i;", 1.2f, 1.5f, 7.9f, 10.4f, false));
  TEST_EQ(1.2f + 1.5f + 7.9f + 10.4f + 19.1f, runfffff("(float f, float g, float h, float i, float j) f + g + h + i + j;", 1.2f, 1.5f, 7.9f, 10.4f, 19.1f, false));


  TEST_EQ(1.f + 1.5f + 7.f + 10.4f + 19.f + 3.14f, runififif("(int i1, float f2, int i3, float f4, int i5, float f6) i1+f2+i3+f4+i5+f6;", 1, 1.5f, 7, 10.4f, 19, 3.14f, false));
  }

TEST_FIXTURE(compile_fixture, parameter_pointer_test)
  {
  int i[3];
  float f[3];
  runpi("(int* i) i[0] = 0; i[1] = 1; i[2] = 2;", &i[0], false);
  TEST_EQ(0, i[0]);
  TEST_EQ(1, i[1]);
  TEST_EQ(2, i[2]);
  runpf("(float* f) f[0] = 0.5; f[1] = 1.5; f[2] = 2.5;", &f[0], false);
  TEST_EQ(0.5f, f[0]);
  TEST_EQ(1.5f, f[1]);
  TEST_EQ(2.5f, f[2]);
  runpi("(int* i) i[0] += 7; i[1] += 4; i[2] += 2;", &i[0], false);
  TEST_EQ(7, i[0]);
  TEST_EQ(5, i[1]);
  TEST_EQ(4, i[2]);
  runpf("(float* f) f[0] += 0.1; f[1] += 0.2; f[2] += 0.3;", &f[0], false);
  TEST_EQ(0.5f + 0.1f, f[0]);
  TEST_EQ(1.5f + 0.2f, f[1]);
  TEST_EQ(2.5f + 0.3f, f[2]);
  runpi("(int* i) i[0] -= 10; i[1] -= 4; i[2] -= 21;", &i[0], false);
  TEST_EQ(-3, i[0]);
  TEST_EQ(1, i[1]);
  TEST_EQ(-17, i[2]);
  runpf("(float* f) f[0] -= 0.1; f[1] -= 0.2; f[2] -= 0.3;", &f[0], false);
  TEST_EQ(0.5f, f[0]);
  TEST_EQ(1.5f, f[1]);
  TEST_EQ(2.5f, f[2]);
  runpi("(int* i) i[0] *= 2;", &i[0], false);
  TEST_EQ(-6, i[0]);
  TEST_EQ(1, i[1]);
  TEST_EQ(-17, i[2]);
  runpf("(float* f) f[1] *= 3.14f;", &f[0], false);
  TEST_EQ(0.5f, f[0]);
  TEST_EQ(1.5f * 3.14f, f[1]);
  TEST_EQ(2.5f, f[2]);
  runpi("(int* i) i[0] /= 2;", &i[0], false);
  TEST_EQ(-3, i[0]);
  TEST_EQ(1, i[1]);
  TEST_EQ(-17, i[2]);
  runpf("(float* f) f[1] /= 3.14f;", &f[0], false);
  TEST_EQ(0.5f, f[0]);
  TEST_EQ(1.5f, f[1]);
  TEST_EQ(2.5f, f[2]);
  }

TEST_FIXTURE(compile_fixture, parameter_dereference_test)
  {
  float f = 3.14f;
  int i = 5;
  TEST_EQ(3.14f, runpf("(float* f) float g = *f; g;", &f, false));
  TEST_EQ(5, runpi("(int* i) int g = *i; g;", &i, false));
  TEST_EQ(10.14f, runpf("(float* f) float g = (1 + (1 + (1 + (1 + (1 + (1 + ( 1 + *f))))))); g;", &f, false));
  TEST_EQ(12, runpi("(int* i) int g = (1 + (1 + (1 + (1 + (1 + (1 + ( 1 + *i))))))); g;", &i, false));

  TEST_EQ(3.14f, runpf("(float* f) float g = f[0]; g;", &f, false));
  TEST_EQ(5, runpi("(int* i) int g = i[0]; g;", &i, false));
  TEST_EQ(10.14f, runpf("(float* f) float g = (1 + (1 + (1 + (1 + (1 + (1 + ( 1 + f[0]))))))); g;", &f, false));
  TEST_EQ(12, runpi("(int* i) int g = (1 + (1 + (1 + (1 + (1 + (1 + ( 1 + i[0]))))))); g;", &i, false));

  TEST_EQ(3.14f*3.14f, runpf("(float* f) *f * *f;", &f, false));
  TEST_EQ(25.f, runpi("(int* i) *i * *i;", &i, false));

  runpf("(float* f) *f += 1.2;", &f, false);
  TEST_EQ(3.14f + 1.2f, f);
  runpf("(float* f) *f = 1.2;", &f, false);
  TEST_EQ(1.2f, f);
  runpf("(float* f) *f -= 1.4;", &f, false);
  TEST_EQ(1.2f-1.4f, f);
  runpf("(float* f) *f *= 2.8;", &f, false);
  TEST_EQ((1.2f - 1.4f)*2.8f, f);
  runpf("(float* f) *f /= 2.5;", &f, false);
  TEST_EQ((1.2f - 1.4f)*2.8f/2.5f, f);

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

TEST_FIXTURE(compile_fixture, inc_dec_test)
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

  int i[3];
  runpi("(int* i) i[0] = 0; i[1] = 3; i[2] = 6; ++i[1];", &i[0], false);
  TEST_EQ(0, i[0]);
  TEST_EQ(4, i[1]);
  TEST_EQ(6, i[2]);

  float f[3];
  runpf("(float* f) f[0] = 0; f[1] = 3; f[2] = 6; ++f[1];", &f[0], false);
  TEST_EQ(0.f, f[0]);
  TEST_EQ(4.f, f[1]);
  TEST_EQ(6.f, f[2]);

  int i1 = 7;
  runpi("(int* i) ++*i;", &i1, false);
  TEST_EQ(8, i1);
  runpi("(int* i) --*i;", &i1, false);
  TEST_EQ(7, i1);

  float f1 = 7.5;
  runpf("(float* i) ++*i;", &f1, false);
  TEST_EQ(8.5f, f1);
  runpf("(float* i) --*i;", &f1, false);
  TEST_EQ(7.5f, f1);
  }

TEST_FIXTURE(compile_fixture, for_loop_test)
  {
  TEST_EQ(1225.f, run("() float f = 0; for (int i = 0; i < 50; ++i) { f += i; } f;", false));
  }


TEST_FIXTURE(compile_fixture, harmonic)
  {
  auto tic = std::chrono::high_resolution_clock::now();
  TEST_EQ_CLOSE(14.3574f, run("() float sum = 0.0;for (int i = 1; i<1000000; ++i) { sum += 1.0/i; } sum;", false), 1e-4);
  //TEST_EQ_CLOSE(15.4037f, run("() float sum = 0.0;for (int i = 1; i<1000000000; ++i) { sum += 1.0/i; } sum;", false), 1e-4);
  auto toc = std::chrono::high_resolution_clock::now();
  auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(toc - tic).count();
  printf("Harmonic timing: %dms\n", ms);
  }

TEST_FIXTURE(compile_fixture, optimize_tests)
  {
  TEST_EQ(0, run("() int i = 0;", false));
  TEST_EQ(0, run("() float f = 0.0;", false));
  TEST_EQ(0, run("() float f = 0;", false));
  }
TEST_FIXTURE(compile_fixture, funccall_tests)
  {
  TEST_EQ(std::sqrt(2.f), run("() sqrt(2.f);", false));
  TEST_EQ(std::sqrt(2.f), run("() sqrt(2.f);", true));

  TEST_EQ(std::sin(0.5f), run("() sin(0.5f);", false));
  TEST_EQ(std::sin(0.5f), run("() sin(0.5f);", true));

  TEST_EQ(std::cos(0.5f), run("() cos(0.5f);", false));
  TEST_EQ(std::cos(0.5f), run("() cos(0.5f);", true));

  TEST_EQ(std::exp(0.5f), run("() exp(0.5f);", false));
  TEST_EQ(std::exp(0.5f), run("() exp(0.5f);", true));

  TEST_EQ(std::log(0.5f), run("() log(0.5f);", false));
  TEST_EQ(std::log(0.5f), run("() log(0.5f);", true));

  TEST_EQ(std::log2(0.5f), run("() log2(0.5f);", false));
  TEST_EQ(std::log2(0.5f), run("() log2(0.5f);", true));

  TEST_EQ(std::fabs(-0.5f), run("() fabs(0.5f);", false));
  TEST_EQ(std::fabs(-0.5f), run("() fabs(0.5f);", true));
  }

TEST_FIXTURE(compile_fixture, rsp_offset_test)
  {
  TEST_EQ_CLOSE(0.f, runf("(float x) float y = 1.0 - x; sin(y*y*3.141592653589793238462643383*4.0);", 1.f), 1e-5);
  TEST_EQ_CLOSE(0.f, runf("(float x) float y = 1.0 - x; sin(y*y*3.141592653589793238462643383*4.0);", 0.f), 1e-5);
  }
*/
COMPILER_END

void run_all_compile_tests()
  {
  using namespace COMPILER;

  compiler_float().test();
  }