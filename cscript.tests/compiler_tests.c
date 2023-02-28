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

  cscript_vector compiled_program = cscript_compile_program(ctxt, &prog);
  cscript_fixnum* res = cscript_run_program(ctxt, &compiled_program);

  TEST_EQ_INT(expected, *res);

  cscript_print_any_error(ctxt);

  TEST_EQ_INT(0, ctxt->number_of_compile_errors);
  TEST_EQ_INT(0, ctxt->number_of_syntax_errors);
  TEST_EQ_INT(0, ctxt->number_of_runtime_errors);

  cscript_compiled_program_destroy(ctxt, &compiled_program);
  destroy_tokens_vector(ctxt, &tokens);
  cscript_program_destroy(ctxt, &prog);
  cscript_close(ctxt);
  }


static void test_compile_flonum_aux_stack(cscript_flonum expected, const char* script, int stack_size)
  {
  cscript_context* ctxt = cscript_open(stack_size);
  cscript_vector tokens = cscript_script2tokens(ctxt, script);
  cscript_program prog = make_program(ctxt, &tokens);

  cscript_vector compiled_program = cscript_compile_program(ctxt, &prog);
  cscript_flonum* res = cast(cscript_flonum*, cscript_run_program(ctxt, &compiled_program));

  TEST_EQ_DOUBLE(expected, *res);

  TEST_EQ_INT(0, ctxt->number_of_compile_errors);
  TEST_EQ_INT(0, ctxt->number_of_syntax_errors);
  TEST_EQ_INT(0, ctxt->number_of_runtime_errors);

  cscript_compiled_program_destroy(ctxt, &compiled_program);
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
  }

void run_all_compiler_tests()
  {
  test_compile_fixnum();
  test_compile_flonum();
  test_compile_term();
  }