#include "cscripttests.h"

#include "cscript/cscript.h"

#include "test_assert.h"

COMPILER_BEGIN

void test_invalid_cscript()
  {
  cscript_environment env;
  std::string error_message;
  std::string script = R"(() this is invalid)";
  auto fun = cscript_function::create(script, env, error_message);
  TEST_EQ(nullptr, fun.get());
  TEST_ASSERT(!error_message.empty());
  }

void test_valid_cscript()
  {
  cscript_environment env;
  std::string error_message;
  std::string script = R"(() 3.0;)";
  auto fun = cscript_function::create(script, env, error_message);
  TEST_ASSERT(fun.get() != nullptr);
  TEST_ASSERT(error_message.empty());
  }

void test_cscript_with_simple_input_parameters()
  {
  cscript_environment env;
  std::string error_message;
  std::string script = R"((int x, int y) x+y;)";
  auto fun = cscript_function::create(script, env, error_message);
  TEST_ASSERT(fun.get() != nullptr);
  TEST_ASSERT(error_message.empty());
  std::vector<cscript_argument> args;
  args.push_back(make_cscript_argument((int64_t)5));
  args.push_back(make_cscript_argument((int64_t)9));
  double result = fun->run(args, env);
  TEST_EQ(14.0, result);
  }

void test_cscript_with_many_integer_input_parameters()
  {
  cscript_environment env;
  std::string error_message;
  std::string script = R"((int a, int b, int c, int d, int e, int f, int g, int h) a+b+c+d+e+f+g+h;)";
  auto fun = cscript_function::create(script, env, error_message);
  TEST_ASSERT(fun.get() != nullptr);
  TEST_ASSERT(error_message.empty());
  std::vector<cscript_argument> args;
  args.push_back(make_cscript_argument((int64_t)1));
  args.push_back(make_cscript_argument((int64_t)2));
  args.push_back(make_cscript_argument((int64_t)3));
  args.push_back(make_cscript_argument((int64_t)4));
  args.push_back(make_cscript_argument((int64_t)5));
  args.push_back(make_cscript_argument((int64_t)6));
  args.push_back(make_cscript_argument((int64_t)7));
  args.push_back(make_cscript_argument((int64_t)8));
  double result = fun->run(args, env);
  TEST_EQ(36, result);
  }

void test_cscript_with_many_float_input_parameters()
  {
  cscript_environment env;
  std::string error_message;
  std::string script = R"((float a, float b, float c, float d, float e, float f, float g, float h) a+b+c+d+e+f+g+h;)";
  auto fun = cscript_function::create(script, env, error_message);
  TEST_ASSERT(fun.get() != nullptr);
  TEST_ASSERT(error_message.empty());
  std::vector<cscript_argument> args;
  args.push_back(make_cscript_argument(1.1));
  args.push_back(make_cscript_argument(2.2));
  args.push_back(make_cscript_argument(3.3));
  args.push_back(make_cscript_argument(4.4));
  args.push_back(make_cscript_argument(5.5));
  args.push_back(make_cscript_argument(6.6));
  args.push_back(make_cscript_argument(7.7));
  args.push_back(make_cscript_argument(8.8));
  double result = fun->run(args, env);
  TEST_EQ(39.6, result);
  }

void test_quicksort()
  {
  cscript_environment env;
  std::string error_message;
  std::string script = R"((int* a, int* stack, int size)
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
)";
  auto fun = cscript_function::create(script, env, error_message);
  TEST_ASSERT(fun.get() != nullptr);
  TEST_ASSERT(error_message.empty());

  uint64_t max_size = 1000;
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

  std::vector<cscript_argument> args;

  args.push_back(make_cscript_argument(a.data()));
  args.push_back(make_cscript_argument(st.data()));
  args.push_back(make_cscript_argument(max_size));

  fun->run(args, env);
  bool array_is_sorted = true;
  for (uint64_t i = 0; i < max_size - 1; ++i)
    array_is_sorted &= a[i] <= a[i + 1];
  TEST_ASSERT(array_is_sorted);
  }

void test_global_variables()
  {
  cscript_environment env;
  std::string error_message;
  std::string script = R"((int x, int y) int $global_value = x+y;)";
  auto fun = cscript_function::create(script, env, error_message);
  TEST_ASSERT(fun.get() != nullptr);
  TEST_ASSERT(error_message.empty());
  std::vector<cscript_argument> args;
  args.push_back(make_cscript_argument((int64_t)5));
  args.push_back(make_cscript_argument((int64_t)9));
  fun->run(args, env);
  std::string script2 = R"((int* result) *result = $global_value;)";
  auto fun2 = cscript_function::create(script2, env, error_message);
  TEST_ASSERT(fun2.get() != nullptr);
  TEST_ASSERT(error_message.empty());
  args.clear();
  int64_t res = 0;
  args.push_back(make_cscript_argument(&res));
  fun2->run(args, env);
  TEST_EQ(14, res);
  }

void test_external_variable_change()
  {
  cscript_environment env;
  std::string error_message;
  std::string script = R"((float* a, float* b, float* p, int* function, int pitch, int velocity, float frequency)
*a = pitch/127.0;
)";
  auto fun = cscript_function::create(script, env, error_message);
  TEST_ASSERT(fun.get() != nullptr);
  TEST_ASSERT(error_message.empty());
  std::vector<cscript_argument> args;
  double a = 0.0;
  double b = 0.0;
  double p = 0.0;
  int64_t f = 0;
  args.push_back(make_cscript_argument((uint64_t)&a));
  args.push_back(make_cscript_argument((uint64_t)&b));
  args.push_back(make_cscript_argument((uint64_t)&p));
  args.push_back(make_cscript_argument((uint64_t)&f));
  args.push_back(make_cscript_argument((uint64_t)10));
  args.push_back(make_cscript_argument((uint64_t)127));
  args.push_back(make_cscript_argument((double)440.0));
  fun->run(args, env);  
  TEST_EQ(10.0/127.0, a);
  }


void test_external_variable_create()
  {
  cscript_environment env;
  std::string error_message;
  std::string script = R"((int* a)
*a = $my_global;
)";
  make_cscript_global_variable("$my_global", (int64_t)3, env);
  auto fun = cscript_function::create(script, env, error_message);
  TEST_ASSERT(fun.get() != nullptr);
  TEST_ASSERT(error_message.empty());
  std::vector<cscript_argument> args;
  int64_t a = 0; 
  args.push_back(make_cscript_argument((uint64_t)&a)); 
  fun->run(args, env);
  TEST_EQ(3, a);
  }

void test_external_variable_create_2()
  {
  cscript_environment env;
  std::string error_message;
  std::string script = R"((float* a)
*a = $my_global;
)";
  make_cscript_global_variable("$my_global", 3.14, env);
  auto fun = cscript_function::create(script, env, error_message);
  TEST_ASSERT(fun.get() != nullptr);
  TEST_ASSERT(error_message.empty());
  std::vector<cscript_argument> args;
  double a = 0.0;
  args.push_back(make_cscript_argument((uint64_t)&a));
  fun->run(args, env);
  TEST_EQ(3.14, a);
  }

namespace
  {
  double add(double a, double b)
    {
    return a+b;
    }
  }

void test_external_functions()
  {
  std::vector<cscript_external_function> externals;
  externals.push_back(make_external_function("add", &add, cscript_return_type::cpp_double, cscript_argument_type::cpp_double, cscript_argument_type::cpp_double));
  cscript_environment env;
  std::string error_message;
  std::string script = R"((float a, float b)
add(a*0.5, b*0.5);
)";
  auto fun = cscript_function::create(script, env, error_message, externals);
  TEST_ASSERT(fun.get() != nullptr);
  TEST_ASSERT(error_message.empty());
  std::vector<cscript_argument> args;  
  args.push_back(make_cscript_argument((double)2.4));
  args.push_back(make_cscript_argument((double)3.6));
  double result = fun->run(args, env);
  TEST_EQ(2.4*0.5+3.6*0.5, result);
  std::string script2 = R"((float a, float b)
add(a*0.5, b*0.5);
a+b;
)";
  auto fun2 = cscript_function::create(script2, env, error_message, externals);
  TEST_ASSERT(fun2.get() != nullptr);
  TEST_ASSERT(error_message.empty());
  result = fun2->run(args, env);
  TEST_EQ(2.4 + 3.6, result);
  }

COMPILER_END


void run_all_cscript_tests()
  {
  using namespace COMPILER;
  test_invalid_cscript();
  test_valid_cscript();
  test_cscript_with_simple_input_parameters();
  test_cscript_with_many_integer_input_parameters();
  test_cscript_with_many_float_input_parameters();
  test_quicksort();
  test_global_variables();
  test_external_variable_change();
  test_external_variable_create();
  test_external_variable_create_2();
  test_external_functions();
  }