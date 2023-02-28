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
  cscript_vector tokens = cscript_script2tokens(ctxt, ";");
  cscript_program prog = make_program(ctxt, &tokens);
  TEST_EQ_INT(1, prog.statements.vector_size);
  cscript_statement* expr = cscript_vector_at(&prog.statements, 0, cscript_statement);
  TEST_EQ_INT(cscript_statement_type_nop, expr->type);
  TEST_EQ_INT(1, expr->statement.nop.line_nr);
  TEST_EQ_INT(1, expr->statement.nop.column_nr);
  destroy_tokens_vector(ctxt, &tokens);
  //cscript_program_destroy(ctxt, &prog);
  cscript_close(ctxt);
  }

void run_all_parser_tests()
  {
  parse_fixnum_1();
  }