#include "compiler_tests.h"
#include "test_assert.h"
#include "cscript/context.h"
#include "cscript/parser.h"
#include "cscript/token.h"
#include "cscript/compiler.h"
#include "cscript/vm.h"
#include "cscript/error.h"
#include "cscript/dump.h"

static void test_compile_aux_stack(cscript_fixnum expected, const char* script, int stack_size, int nr_parameters, void* pars, int output_type)
  {
  cscript_context* ctxt = cscript_open(stack_size);
  cscript_vector tokens = cscript_script2tokens(ctxt, script);
  cscript_program prog = make_program(ctxt, &tokens);

  cscript_function* compiled_program = cscript_compile_program(ctxt, &prog);

  // fill stack with parameters
  for (int i = 0; i < nr_parameters; ++i)
    {
    cscript_fixnum* fx = cscript_vector_begin(&ctxt->stack, cscript_fixnum) + i;
    *fx = *(cast(cscript_fixnum*, pars) + i);
    }

  cscript_fixnum* res = cscript_run(ctxt, compiled_program);

  if (output_type == cscript_number_type_fixnum)
    {    
    TEST_EQ_INT(expected, *res);
    }
  else
    {
    cscript_flonum* resf = cast(cscript_flonum*, res);
    cscript_flonum expectedf = *cast(cscript_flonum*, &expected);
    TEST_EQ_DOUBLE(expectedf, *resf);
    }

  cscript_print_any_error(ctxt);

  TEST_EQ_INT(0, ctxt->number_of_compile_errors);
  TEST_EQ_INT(0, ctxt->number_of_syntax_errors);
  TEST_EQ_INT(0, ctxt->number_of_runtime_errors);

  cscript_function_free(ctxt, compiled_program);
  destroy_tokens_vector(ctxt, &tokens);
  cscript_program_destroy(ctxt, &prog);
  cscript_close(ctxt);
  }

static void test_compile_fixnum_aux_stack(cscript_fixnum expected, const char* script, int stack_size, int nr_parameters, void* pars)
  {
  test_compile_aux_stack(expected, script, stack_size, nr_parameters, pars, cscript_number_type_fixnum);
  }

static void test_compile_flonum_aux_stack(cscript_flonum expected, const char* script, int stack_size, int nr_parameters, void* pars)
  {
  test_compile_aux_stack(*cast(cscript_fixnum*, &expected), script, stack_size, nr_parameters, pars, cscript_number_type_flonum);
  }

static void test_compile_fixnum_aux(cscript_fixnum expected, const char* script)
  {
  test_compile_fixnum_aux_stack(expected, script, 256, 0, NULL);
  }

static void test_compile_flonum_aux(cscript_flonum expected, const char* script)
  {
  test_compile_flonum_aux_stack(expected, script, 256, 0, NULL);
  }

static void test_compile_fixnum_pars_aux(cscript_fixnum expected, const char* script, int nr_parameters, void* pars)
  {
  test_compile_fixnum_aux_stack(expected, script, 256, nr_parameters, pars);
  }

static void test_compile_flonum_pars_aux(cscript_flonum expected, const char* script, int nr_parameters, void* pars)
  {
  test_compile_flonum_aux_stack(expected, script, 256, nr_parameters, pars);
  }

static void test_compile_fixnum()
  {
  test_compile_fixnum_aux(5, "5;");
  test_compile_fixnum_aux(2, "7; 3; 2;");
  test_compile_fixnum_aux(20, "20;");
  }

static void test_compile_flonum()
  {
  test_compile_flonum_aux(5.1, "5.1;");
  test_compile_flonum_aux(3.14159, "3.14159;");
  test_compile_flonum_aux(3.0, "1.0; 2.0; 3.0;");
  }

static void test_compile_term()
  {
  test_compile_fixnum_aux(12, "5 + 7;");
  test_compile_flonum_aux(12.15, "5.15 + 7;");
  test_compile_flonum_aux(12.29, "5.15 + 7.14;");
  test_compile_flonum_aux(12.14, "5 + 7.14;");
  }

static void test_compile_expr()
  {
  test_compile_fixnum_aux(26, "5 + 7 * 3;");
  test_compile_flonum_aux(27.5, "5 + 7.5 * 3;");
  test_compile_flonum_aux(-2.0, "(5%2 - 7)/3.0;");
  test_compile_fixnum_aux(1, "2 < 3;");
  test_compile_fixnum_aux(0, "4 < 3;");
  test_compile_fixnum_aux(1, "2 < 3.2;");
  test_compile_fixnum_aux(0, "4.1 < 3;");
  test_compile_fixnum_aux(1, "2.0 < 3.2;");
  test_compile_fixnum_aux(0, "4.1 < 3.5;");
  test_compile_fixnum_aux(1, "1 == 1.0;");
  test_compile_fixnum_aux(0, "1 != 1.0;");
  test_compile_fixnum_aux(1, "1 >= 1.0;");
  test_compile_fixnum_aux(1, "1 <= 1.0;");
  test_compile_fixnum_aux(1, "1 >= 0.0;");
  test_compile_fixnum_aux(0, "1 <= 0.0;");
  test_compile_fixnum_aux(1, "1 > 0;");

  test_compile_flonum_aux(-9.0, "3.0 * (2 / 0.5 - 7);");
  test_compile_flonum_aux(9.0, "-3.0 * (2 / 0.5 - 7);");
  test_compile_fixnum_aux(135, "110+25;");
  test_compile_fixnum_aux(-135, "-(110+25);");

  test_compile_flonum_aux(5.5, "3.0+2.5;");
  test_compile_flonum_aux(-9.0, "3.0*(2/0.5-7);");
  test_compile_flonum_aux(9.0, "-3.0*(2/0.5-7);");
  test_compile_flonum_aux(55.0, "1.0 + 2.0 + 3.0 + 4.0 + 5.0 + 6.0 + 7.0 + 8.0 + 9.0 + 10.0;");
  test_compile_flonum_aux(55.0, "1.0 + (2.0 + (3.0 + (4.0 + (5.0 + (6.0 + (7.0 + (8.0 + (9.0 + 10.0))))))));");
  test_compile_fixnum_aux(55, "1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10;");
  test_compile_fixnum_aux(55, "1 + (2 + (3 + (4 + (5 + (6 + (7 + (8 + (9 + 10))))))));");
  test_compile_flonum_aux(-53.0, "1.0 - 2.0 - 3.0 - 4.0 - 5.0 - 6.0 - 7.0 - 8.0 - 9.0 - 10.0;");
  test_compile_flonum_aux(-5.0, "1.0 - (2.0 - (3.0 - (4.0 - (5.0 - (6.0 - (7.0 - (8.0 - (9.0 - 10.0))))))));");
  test_compile_fixnum_aux(-53, "1 - 2 - 3 - 4 - 5 - 6 - 7 - 8 - 9 - 10;");
  test_compile_fixnum_aux(-5, "1 - (2 - (3 - (4 - (5 - (6 - (7 - (8 - (9 - 10))))))));");
  test_compile_flonum_aux(3628800.0, "1.0 * 2.0 * 3.0 * 4.0 * 5.0 * 6.0 * 7.0 * 8.0 * 9.0 * 10.0;");
  test_compile_flonum_aux(3628800.0, "1.0 * (2.0 * (3.0 * (4.0 * (5.0 * (6.0 * (7.0 * (8.0 * (9.0 * 10.0))))))));");
  test_compile_fixnum_aux(3628800, "1 * 2 * 3 * 4 * 5 * 6 * 7 * 8 * 9 * 10;");
  test_compile_fixnum_aux(3628800, "1 * (2 * (3 * (4 * (5 * (6 * (7 * (8 * (9 * 10))))))));");
  test_compile_flonum_aux(0.125, "1.0 / 2.0 / 4.0;");
  test_compile_fixnum_aux(1, "5 / 3;");
  test_compile_fixnum_aux(2, "6 / 3;");
  test_compile_fixnum_aux(2, "8 / 2 / 2;");
  test_compile_fixnum_aux(32, "1024 / 2 / 2 / 2 / 2 / 2;");
  test_compile_fixnum_aux(32, " ((((1024 / 2) / 2) / 2) / 2) / 2;");
  test_compile_fixnum_aux(8, "1024 / (512 / (256 / (128 / (64 / 32))));");


  test_compile_fixnum_aux(0, "5.0 < 3.0;");
  test_compile_fixnum_aux(1, "3.0 < 5.0;");
  test_compile_fixnum_aux(0, "5 < 3;");
  test_compile_fixnum_aux(1, "3 < 5;");
  test_compile_flonum_aux(0, "(1.0*(1.0*(1.0*(1.0*(5.0 < 3.0)))));");
  test_compile_flonum_aux(1, "(1.0*(1.0*(1.0*(1.0*(3.0 < 5.0)))));");
  test_compile_flonum_aux(0, "(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 < 3.0))))));");
  test_compile_flonum_aux(1, "(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 < 5.0))))));");
  test_compile_flonum_aux(0, "(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 < 3.0)))))));");
  test_compile_flonum_aux(1, "(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 < 5.0)))))));");
  test_compile_fixnum_aux(1, "(1 * (3 < 5));");
  test_compile_fixnum_aux(1, "(1 * (1 * (3 < 5)));");
  test_compile_fixnum_aux(0, "(1 * (5 < 3));");
  test_compile_fixnum_aux(0, "(1 * (1 * (5 < 3)));");
  test_compile_flonum_aux(1, "(1.0*(1.0*(1.0*(1.0*(5.0 > 3.0)))));");
  test_compile_flonum_aux(0, "(1.0*(1.0*(1.0*(1.0*(3.0 > 5.0)))));");
  test_compile_flonum_aux(1, "(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 > 3.0))))));");
  test_compile_flonum_aux(0, "(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 > 5.0))))));");
  test_compile_flonum_aux(1, "(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 > 3.0)))))));");
  test_compile_flonum_aux(0, "(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 > 5.0)))))));");
  test_compile_fixnum_aux(1, "5 > 3;");

  test_compile_fixnum_aux(0, "3 > 5;");
  test_compile_fixnum_aux(0, "(1 * (3 > 5));");
  test_compile_fixnum_aux(0, "(1 * (1 * (3 > 5)));");
  test_compile_fixnum_aux(1, "(1 * (5 > 3));");
  test_compile_fixnum_aux(1, "(1 * (1 * (5 > 3)));");
  test_compile_fixnum_aux(0, "5.0 <= 3.0;");
  test_compile_fixnum_aux(1, "3.0 <= 5.0;");
  test_compile_flonum_aux(0, "(1.0*(1.0*(1.0*(1.0*(5.0 <= 3.0)))));");
  test_compile_flonum_aux(1, "(1.0*(1.0*(1.0*(1.0*(3.0 <= 5.0)))));");
  test_compile_flonum_aux(0, "(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 <= 3.0))))));");
  test_compile_flonum_aux(1, "(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 <= 5.0))))));");
  test_compile_flonum_aux(0, "(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 <= 3.0)))))));");
  test_compile_flonum_aux(1, "(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 <= 5.0)))))));");
  test_compile_fixnum_aux(0, "5 <= 3;");
  test_compile_fixnum_aux(1, "3 <= 5;");
  test_compile_fixnum_aux(1, "(1 * (3 <= 5));");
  test_compile_fixnum_aux(1, "(1 * (1 * (3 <= 5)));");
  test_compile_fixnum_aux(0, "(1 * (5 <= 3));");

  test_compile_fixnum_aux(0, "(1 * (1 * (5 <= 3)));");
  test_compile_fixnum_aux(1, "5 >= 3;");
  test_compile_fixnum_aux(0, "3 >= 5;");
  test_compile_fixnum_aux(0, "(1 * (3 >= 5));");
  test_compile_fixnum_aux(0, "(1 * (1 * (3 >= 5)));");
  test_compile_fixnum_aux(1, "(1 * (5 >= 3));");
  test_compile_fixnum_aux(1, "(1 * (1 * (5 >= 3)));");
  test_compile_fixnum_aux(1, "5.0 >= 3.0;");
  test_compile_fixnum_aux(0, "3.0 >= 5.0;");
  test_compile_flonum_aux(1, "(1.0*(1.0*(1.0*(1.0*(5.0 >= 3.0)))));");
  test_compile_flonum_aux(0, "(1.0*(1.0*(1.0*(1.0*(3.0 >= 5.0)))));");
  test_compile_flonum_aux(1, "(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 >= 3.0))))));");
  test_compile_flonum_aux(0, "(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 >= 5.0))))));");
  test_compile_flonum_aux(1, "(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 >= 3.0)))))));");
  test_compile_flonum_aux(0, "(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 >= 5.0)))))));");
  test_compile_fixnum_aux(0, "5 == 3;");
  test_compile_fixnum_aux(1, "3 == 3;");
  test_compile_fixnum_aux(0, "(1 * (3 == 5));");
  test_compile_fixnum_aux(0, "(1 * (1 * (3 == 5)));");
  test_compile_fixnum_aux(1, "(1 * (3 == 3));");
  test_compile_fixnum_aux(1, "(1 * (1 * (3 == 3)));");
  test_compile_fixnum_aux(0, "5.0 == 3.0;");
  test_compile_fixnum_aux(1, "3.0 == 3.0;");
  test_compile_flonum_aux(0, "(1.0*(1.0*(1.0*(1.0*(5.0 == 3.0)))));");
  test_compile_flonum_aux(1, "(1.0*(1.0*(1.0*(1.0*(3.0 == 3.0)))));");
  test_compile_flonum_aux(0, "(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 == 3.0))))));");
  test_compile_flonum_aux(1, "(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 == 3.0))))));");
  test_compile_flonum_aux(0, "(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 == 3.0)))))));");
  test_compile_flonum_aux(1, "(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 == 3.0)))))));");
  test_compile_fixnum_aux(1, "5 != 3;");
  test_compile_fixnum_aux(0, "3 != 3;");
  test_compile_fixnum_aux(1, "(1 * (3 != 5));");
  test_compile_fixnum_aux(1, "(1 * (1 * (3 != 5)));");
  test_compile_fixnum_aux(0, "(1 * (3 != 3));");
  test_compile_fixnum_aux(0, "(1 * (1 * (3 != 3)));");
  test_compile_fixnum_aux(1, "5.0 != 3.0;");
  test_compile_fixnum_aux(0, "3.0 != 3.0;");
  test_compile_flonum_aux(1, "(1.0*(1.0*(1.0*(1.0*(5.0 != 3.0)))));");
  test_compile_flonum_aux(0, "(1.0*(1.0*(1.0*(1.0*(3.0 != 3.0)))));");
  test_compile_flonum_aux(1, "(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 != 3.0))))));");
  test_compile_flonum_aux(0, "(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 != 3.0))))));");
  test_compile_flonum_aux(1, "(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(5.0 != 3.0)))))));");
  test_compile_flonum_aux(0, "(1.0*(1.0*(1.0*(1.0*(1.0*(1.0*(3.0 != 3.0)))))));");
  }

static void test_compile_named_fixnum()
  {
  test_compile_fixnum_aux(3, "int i = 3; i;");
  test_compile_fixnum_aux(5, "int i = 5.17; i;");
  test_compile_fixnum_aux(6, "int i = 3; int k = 6; int l = 9; k;");
  test_compile_fixnum_aux(1512, "int i = 3*7*8*9; i;");
  test_compile_fixnum_aux(1512, "int i = 3.0*7.0*8.0*9.0; i;");
  }

static void test_compile_named_flonum()
  {
  test_compile_flonum_aux(3.14, "float f = 3.14; f;");
  test_compile_flonum_aux(5.0, "float f = 5; f;");
  test_compile_flonum_aux(1512.0, "float f = 3.0*7.0*8.0*9.0; f;");
  test_compile_flonum_aux(1512.0, "float f = 3*7*8*9; f;");
  }

static void test_assigment()
  {
  test_compile_flonum_aux(4.1415000000000006, "float f; int i; f = 3.1415; i = 1; f+i;");
  test_compile_flonum_aux(4.1415000000000006, "float f; int i; f = 3.1415; i = 1; f+=i;f;");
  test_compile_fixnum_aux(4, "float f; int i; f = 3.1415; i = 1; i+=f;i;");
  test_compile_flonum_aux(2.1415, "float f; int i; f = 3.1415; i = 1; f-=i;f;");
  test_compile_fixnum_aux(-2, "float f; int i; f = 3.1415; i = 1; i-=f;i;");
  test_compile_flonum_aux(6.2830, "float f; int i; f = 3.1415; i = 2; f*=i;f;");
  test_compile_fixnum_aux(6, "float f; int i; f = 3.1415; i = 2; i*=f;i;");
  test_compile_flonum_aux(1.5707500000000001, "float f; int i; f = 3.1415; i = 2; f/=i;f;");
  test_compile_fixnum_aux(3, "float f; int i; f = 3.1415; i = 9; i/=f;i;");
  test_compile_fixnum_aux(4, "int i = 9; i/=(1+1);i;");
  }

static void test_array()
  {
  test_compile_flonum_aux(1.0, "float f[3];\nf[0] = 1;f[0];");
  test_compile_flonum_aux(6.0, "float f[3];\nf[0] = 1;\nf[1] = 2;\nf[2] = 3;\nf[0]*f[1]*f[2];");
  test_compile_fixnum_aux(6, "int f[3];\nf[0] = 1;\nf[1] = 2;\nf[2] = 3;\nf[0]*f[1]*f[2];");
  test_compile_flonum_aux(1.0 + 3.14, "float f[10];\nf[8] = 1;\nf[8] += 3.14;f[8];");
  test_compile_fixnum_aux(4, "int f[10];\nf[8] = 1;\nf[8] += 3;f[8];");
  test_compile_flonum_aux(1.0 - 3.14, "float f[10];\nf[8] = 1;\nf[8] -= 3.14;f[8];");
  test_compile_fixnum_aux(-2, "int f[10];\nf[8] = 1;\nf[8] -= 3;f[8];");
  test_compile_flonum_aux(6.28, "float f[10];\nf[8] = 2;\nf[8] *= 3.14;f[8];");
  test_compile_fixnum_aux(6, "int f[10];\nf[8] = 2;\nf[8] *= 3;f[8];");
  test_compile_flonum_aux(15.8 / 2.15, "float f[10];\nf[8] = 15.8;\nf[8] /= 2.15;f[8];");
  test_compile_fixnum_aux(3, "int f[10];\nf[8] = 10;\nf[8] /= 3;f[8];");
  }

static void test_comment()
  {
  const char* script =
    "int i[3]; // make an array of 3 integers\n"
    "i[1] = 8; // assign 8 to index 1 position\n"    
    "i[1] /= 2; // divide index 1 position by 2\n"
    "i[1]; // return the result in index 1";
  test_compile_fixnum_aux(4, script);
  const char* script2 =
  "int i[3]; // make an array of 3 integers\n"
  "i[1] = 8; // assign 8 to index 1 position\n"
  "i[1];\n"
  "/*\n"
  "i[1] /= 2; // divide index 1 position by 2\n"
  "i[1]; // return the result in index 1\n"
  "*/\n";
  test_compile_fixnum_aux(8, script2);
  }

static void test_parameter()
  {
  cscript_fixnum i;
  i = 3;
  test_compile_fixnum_pars_aux(4, "(int i) i + 1;", 1, &i);
  }

void run_all_compiler_tests()
  {
  test_compile_fixnum();
  test_compile_flonum();
  test_compile_term();
  test_compile_expr();
  test_compile_named_fixnum();
  test_compile_named_flonum();
  test_assigment();
  test_array();
  test_comment();
  test_parameter();
  }