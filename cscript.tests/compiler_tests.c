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

static cscript_fixnum convert_to_fx(cscript_flonum fl)
  {
  return *cast(cscript_fixnum*, &fl);
  }

static void test_parameter()
  {
  cscript_fixnum pars_list[6] = {0, 0, 0, 0, 0, 0};
  pars_list[0] = 3;
  test_compile_fixnum_pars_aux(4, "(int i) i + 1;", 1, pars_list);
  pars_list[1] = 7;
  test_compile_fixnum_pars_aux(10, "(int i, int j) i + j;", 2, pars_list);
  pars_list[2] = 5;
  test_compile_fixnum_pars_aux(15, "(int i, int j, int k) i + j + k;", 3, pars_list);
  pars_list[3] = 20;
  test_compile_fixnum_pars_aux(35, "(int i, int j, int k, int l) i + j + k + l;", 4, pars_list);
  pars_list[4] = 100;
  test_compile_fixnum_pars_aux(135, "(int i, int j, int k, int l, int m) i + j + k + l + m;", 5, pars_list);

  pars_list[0] = convert_to_fx(1.2);
  test_compile_flonum_pars_aux(1.2 + 1.5, "(float f) f + 1.5;", 1, pars_list);
  pars_list[1] = convert_to_fx(1.5);
  test_compile_flonum_pars_aux(1.2 + 1.5, "(float f, float g) f + g;", 2, pars_list);
  pars_list[2] = convert_to_fx(7.9);
  test_compile_flonum_pars_aux(1.2 + 1.5 + 7.9, "(float f, float g, float h) f + g + h;", 3, pars_list);
  pars_list[3] = convert_to_fx(10.4);
  test_compile_flonum_pars_aux(1.2 + 1.5 + 7.9 + 10.4, "(float f, float g, float h, float i) f + g + h + i;", 4, pars_list);
  pars_list[4] = convert_to_fx(19.1);
  test_compile_flonum_pars_aux(1.2 + 1.5 + 7.9 + 10.4 + 19.1, "(float f, float g, float h, float i, float j) f + g + h + i + j;", 5, pars_list);

  pars_list[0] = 1;
  pars_list[1] = convert_to_fx(1.5);
  pars_list[2] = 7;
  pars_list[3] = convert_to_fx(10.4);
  pars_list[4] = 19;
  pars_list[5] = convert_to_fx(3.14);
  test_compile_flonum_pars_aux(1.0 + 1.5 + 7.0 + 10.4 + 19.0 + 3.14, "(int i1, float f2, int i3, float f4, int i5, float f6) i1 + f2 + i3 + f4 + i5 + f6; ", 6, pars_list);
  }

static void test_parameter_pointer()
  {
  cscript_fixnum i[3] = {9999, 9999, 9999};
  cscript_flonum f[3] = {9999.0, 9999.0, 9999.0};
  cscript_fixnum pars_list[6] = { 0, 0, 0, 0, 0, 0 };
  pars_list[0] = &i[0];
  test_compile_fixnum_pars_aux(15, "(int* i) i[0] = 0; i[1] = 1; i[2] = 2; 15;", 1, pars_list);
  TEST_EQ_INT(0, i[0]);
  TEST_EQ_INT(1, i[1]);
  TEST_EQ_INT(2, i[2]);

  pars_list[0] = &f[0];
  test_compile_flonum_pars_aux(15.0, "(float* f)  f[0] = 0.5; f[1] = 1.5; f[2] = 2.5; 15.0;", 1, pars_list);
  TEST_EQ_DOUBLE(0.5, f[0]);
  TEST_EQ_DOUBLE(1.5, f[1]);
  TEST_EQ_DOUBLE(2.5, f[2]);

  pars_list[0] = &i[0];
  test_compile_fixnum_pars_aux(15, "(int* i) i[0] += 7; i[1] += 4; i[2] += 2; 15;", 1, pars_list);
  TEST_EQ_INT(7, i[0]);
  TEST_EQ_INT(5, i[1]);
  TEST_EQ_INT(4, i[2]);

  pars_list[0] = &f[0];
  test_compile_flonum_pars_aux(15.0, "(float* f) f[0] += 0.1; f[1] += 0.2; f[2] += 0.3; 15.0;", 1, pars_list);
  TEST_EQ_DOUBLE(0.5 + 0.1, f[0]);
  TEST_EQ_DOUBLE(1.5 + 0.2, f[1]);
  TEST_EQ_DOUBLE(2.5 + 0.3, f[2]);

  pars_list[0] = &i[0];
  test_compile_fixnum_pars_aux(15, "(int* i) i[0] -= 10; i[1] -= 4; i[2] -= 21; 15;", 1, pars_list);
  TEST_EQ_INT(-3, i[0]);
  TEST_EQ_INT(1, i[1]);
  TEST_EQ_INT(-17, i[2]);

  pars_list[0] = &f[0];
  test_compile_flonum_pars_aux(15.0, "(float* f) f[0] -= 0.1; f[1] -= 0.2; f[2] -= 0.3; 15.0;", 1, pars_list);
  TEST_EQ_DOUBLE(0.5, f[0]);
  TEST_EQ_DOUBLE(1.5, f[1]);
  TEST_EQ_DOUBLE(2.5, f[2]);

  pars_list[0] = &i[0];
  test_compile_fixnum_pars_aux(15, "(int* i) i[0] *= 2;; 15;", 1, pars_list);
  TEST_EQ_INT(-6, i[0]);
  TEST_EQ_INT(1, i[1]);
  TEST_EQ_INT(-17, i[2]);

  pars_list[0] = &f[0];
  test_compile_flonum_pars_aux(15.0, "(float* f)  f[1] *= 3.14f; 15.0;", 1, pars_list);
  TEST_EQ_DOUBLE(0.5, f[0]);
  TEST_EQ_DOUBLE(1.5 * 3.14, f[1]);
  TEST_EQ_DOUBLE(2.5, f[2]);

  pars_list[0] = &i[0];
  test_compile_fixnum_pars_aux(15, "(int* i) i[0] /= 2; 15;", 1, pars_list);
  TEST_EQ_INT(-3, i[0]);
  TEST_EQ_INT(1, i[1]);
  TEST_EQ_INT(-17, i[2]);

  pars_list[0] = &f[0];
  test_compile_flonum_pars_aux(15.0, "(float* f)  f[1] /= 3.14f; 15.0;", 1, pars_list);
  TEST_EQ_DOUBLE(0.5, f[0]);
  TEST_EQ_DOUBLE(1.5, f[1]);
  TEST_EQ_DOUBLE(2.5, f[2]);

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
  test_parameter_pointer();
  }