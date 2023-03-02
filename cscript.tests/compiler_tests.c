#include "compiler_tests.h"
#include "test_assert.h"
#include "cscript/context.h"
#include "cscript/parser.h"
#include "cscript/token.h"
#include "cscript/compiler.h"
#include "cscript/vm.h"
#include "cscript/error.h"
#include "cscript/dump.h"


static void test_compile_fixnum_aux_stack(cscript_fixnum expected, const char* script, int stack_size)
  {
  cscript_context* ctxt = cscript_open(stack_size);
  cscript_vector tokens = cscript_script2tokens(ctxt, script);
  cscript_program prog = make_program(ctxt, &tokens);

  cscript_function* compiled_program = cscript_compile_program(ctxt, &prog);
  cscript_fixnum* res = cscript_run(ctxt, compiled_program);

  TEST_EQ_INT(expected, *res);

  cscript_print_any_error(ctxt);

  TEST_EQ_INT(0, ctxt->number_of_compile_errors);
  TEST_EQ_INT(0, ctxt->number_of_syntax_errors);
  TEST_EQ_INT(0, ctxt->number_of_runtime_errors);

  cscript_function_free(ctxt, compiled_program);
  destroy_tokens_vector(ctxt, &tokens);
  cscript_program_destroy(ctxt, &prog);
  cscript_close(ctxt);
  }


static void test_compile_flonum_aux_stack(cscript_flonum expected, const char* script, int stack_size)
  {
  cscript_context* ctxt = cscript_open(stack_size);
  cscript_vector tokens = cscript_script2tokens(ctxt, script);
  cscript_program prog = make_program(ctxt, &tokens);

  cscript_function* compiled_program = cscript_compile_program(ctxt, &prog);
  cscript_flonum* res = cast(cscript_flonum*, cscript_run(ctxt, compiled_program));

  TEST_EQ_DOUBLE(expected, *res);

  TEST_EQ_INT(0, ctxt->number_of_compile_errors);
  TEST_EQ_INT(0, ctxt->number_of_syntax_errors);
  TEST_EQ_INT(0, ctxt->number_of_runtime_errors);

  cscript_function_free(ctxt, compiled_program);
  destroy_tokens_vector(ctxt, &tokens);
  cscript_program_destroy(ctxt, &prog);
  cscript_close(ctxt);
  }

static void test_compile_fixnum_aux(cscript_fixnum expected, const char* script)
  {
  test_compile_fixnum_aux_stack(expected, script, 256);
  }

static void test_compile_flonum_aux(cscript_flonum expected, const char* script)
  {
  test_compile_flonum_aux_stack(expected, script, 256);
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

void run_all_compiler_tests()
  {
  test_compile_fixnum();
  test_compile_flonum();
  test_compile_term();
  test_compile_expr();
  test_compile_named_fixnum();
  test_compile_named_flonum();
  }