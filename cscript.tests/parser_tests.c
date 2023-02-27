#include "test_assert.h"
#include "parser_tests.h"

#include "cscript/token.h"
#include "cscript/context.h"
#include "cscript/stream.h"
#include "cscript/parser.h"
#include "cscript/error.h"

static void parse_fixnum_1()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector tokens = cscript_script2tokens(ctxt, "5");
  //cscript_program prog = make_program(ctxt, &tokens);
  //TEST_EQ_INT(1, prog.expressions.vector_size);
  //cscript_expression* expr = cscript_vector_at(&prog.expressions, 0, cscript_expression);
  //TEST_EQ_INT(cscript_type_literal, expr->type);
  //TEST_EQ_INT(cscript_type_fixnum, expr->expr.lit.type);
  //TEST_EQ_INT(5, expr->expr.lit.lit.fx.value);
  //TEST_EQ_INT(1, expr->expr.lit.lit.fx.line_nr);
  //TEST_EQ_INT(1, expr->expr.lit.lit.fx.column_nr);
  destroy_tokens_vector(ctxt, &tokens);
  //cscript_program_destroy(ctxt, &prog);
  cscript_close(ctxt);
  }

void run_all_parser_tests()
  {
  parse_fixnum_1();
  }