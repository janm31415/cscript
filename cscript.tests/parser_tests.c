#include "test_assert.h"
#include "parser_tests.h"

#include "cscript/token.h"
#include "cscript/context.h"
#include "cscript/stream.h"
#include "cscript/parser.h"
#include "cscript/error.h"
#include "cscript/dump.h"

static void parse_semicolon()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector tokens = cscript_script2tokens(ctxt, ";");
  cscript_program prog = make_program(ctxt, &tokens);
  TEST_EQ_INT(1, prog.statements.vector_size);
  cscript_statement* expr = cscript_vector_at(&prog.statements, 0, cscript_statement);
  TEST_EQ_INT(cscript_statement_type_nop, expr->type);
  TEST_EQ_INT(1, expr->statement.nop.line_nr);
  TEST_EQ_INT(1, expr->statement.nop.column_nr);

  cscript_dump_visitor* dumper = cscript_dump_visitor_new(ctxt);
  cscript_visit_program(ctxt, dumper->visitor, &prog);
  TEST_EQ_STRING(";\n", dumper->s.string_ptr);
  cscript_dump_visitor_free(ctxt, dumper);

  destroy_tokens_vector(ctxt, &tokens);
  cscript_program_destroy(ctxt, &prog);
  cscript_close(ctxt);
  }

static void parse_number_1()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector tokens = cscript_script2tokens(ctxt, "5;");
  cscript_program prog = make_program(ctxt, &tokens);
  TEST_EQ_INT(1, prog.statements.vector_size);
  cscript_statement* expr = cscript_vector_at(&prog.statements, 0, cscript_statement);
  TEST_EQ_INT(cscript_statement_type_expression, expr->type);
  TEST_EQ_INT(1, expr->statement.expr.line_nr);
  TEST_EQ_INT(1, expr->statement.expr.column_nr);  
  destroy_tokens_vector(ctxt, &tokens);
  cscript_program_destroy(ctxt, &prog);
  cscript_close(ctxt);
  }

static void parse_number_2()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector tokens = cscript_script2tokens(ctxt, "3.14;");
  cscript_program prog = make_program(ctxt, &tokens);
  TEST_EQ_INT(1, prog.statements.vector_size);
  cscript_statement* expr = cscript_vector_at(&prog.statements, 0, cscript_statement);
  TEST_EQ_INT(cscript_statement_type_expression, expr->type);
  TEST_EQ_INT(1, expr->statement.expr.line_nr);
  TEST_EQ_INT(1, expr->statement.expr.column_nr);
  destroy_tokens_vector(ctxt, &tokens);
  cscript_program_destroy(ctxt, &prog);
  cscript_close(ctxt);
  }

static void parse_term()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector tokens = cscript_script2tokens(ctxt, "1+2+3.14;");
  cscript_program prog = make_program(ctxt, &tokens);
  TEST_EQ_INT(1, prog.statements.vector_size);
  cscript_statement* expr = cscript_vector_at(&prog.statements, 0, cscript_statement);
  TEST_EQ_INT(cscript_statement_type_expression, expr->type);
  TEST_EQ_INT(1, expr->statement.expr.line_nr);
  TEST_EQ_INT(1, expr->statement.expr.column_nr);
  destroy_tokens_vector(ctxt, &tokens);
  cscript_program_destroy(ctxt, &prog);
  cscript_close(ctxt);
  }

static void parse_expression()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector tokens = cscript_script2tokens(ctxt, "(1+2+3.14)*27 < 3.14-5%3;");
  cscript_program prog = make_program(ctxt, &tokens);
  TEST_EQ_INT(1, prog.statements.vector_size);
  cscript_statement* expr = cscript_vector_at(&prog.statements, 0, cscript_statement);
  TEST_EQ_INT(cscript_statement_type_expression, expr->type);
  TEST_EQ_INT(1, expr->statement.expr.line_nr);
  TEST_EQ_INT(1, expr->statement.expr.column_nr);

  cscript_dump_visitor* dumper = cscript_dump_visitor_new(ctxt);
  cscript_visit_program(ctxt, dumper->visitor, &prog);
  TEST_EQ_STRING("(1+2+3.140000)*27<3.140000-5%3;\n", dumper->s.string_ptr);
  cscript_dump_visitor_free(ctxt, dumper);

  destroy_tokens_vector(ctxt, &tokens);
  cscript_program_destroy(ctxt, &prog);
  cscript_close(ctxt);
  }

void run_all_parser_tests()
  {
  parse_semicolon(); 
  parse_number_1();
  parse_number_2();
  parse_term();
  parse_expression();
  }