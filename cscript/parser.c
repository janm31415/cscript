#include "parser.h"
#include "error.h"
#include "context.h"

#include <string.h>
#include <stdlib.h>

#define CSCRIPT_MAX_SYNTAX_ERRORS 5


static cscript_string make_filename(cscript_context* ctxt)
  {
  cscript_string s;
  if (ctxt->filenames_list.vector_size > 0)
    {
    cscript_string_copy(ctxt, &s, cscript_vector_back(&ctxt->filenames_list, cscript_string));
    }
  else
    {
    cscript_string_init(ctxt, &s, "");
    }
  return s;
  }

static token make_bad_token()
  {
  token t;
  t.type = CSCRIPT_T_BAD;
  return t;
  }

token popped_token;
token last_token;

static void invalidate_popped()
  {
  popped_token = make_bad_token();
  }

static cscript_parsed_statement make_nop()
  {
  cscript_parsed_nop nop;
  nop.column_nr = -1;
  nop.line_nr = -1;
  nop.filename = make_null_string();
  cscript_parsed_statement stmt;
  stmt.type = cscript_statement_type_nop;
  stmt.statement.nop = nop;
  return stmt;
  }

static int current_token_type(token** token_it, token** token_it_end)
  {
  if (*token_it == *token_it_end)
    return CSCRIPT_T_BAD;
  return (*token_it)->type;
  }

cscript_parsed_statement cscript_make_statement(cscript_context* ctxt, token** token_it, token** token_it_end)
  {
  if (ctxt->number_of_syntax_errors > CSCRIPT_MAX_SYNTAX_ERRORS)
    return make_nop();
  if (*token_it == *token_it_end)
    {
    cscript_string* fn = NULL;
    if (ctxt->filenames_list.vector_size > 0)
      fn = cscript_vector_back(&ctxt->filenames_list, cscript_string);
    cscript_syntax_error_cstr(ctxt, CSCRIPT_ERROR_NO_TOKENS, last_token.line_nr, last_token.column_nr, fn, "");
    return make_nop();
    }

  switch (current_token_type(token_it, token_it_end))
    {
    default:
      cscript_throw(ctxt, CSCRIPT_ERROR_NOT_IMPLEMENTED);

    }

  return make_nop();
  }

cscript_program make_program(cscript_context* ctxt, cscript_vector* tokens)
  {
  last_token = make_bad_token();
  cscript_syntax_errors_clear(ctxt);
  invalidate_popped();
  cscript_program prog;
  cscript_vector_init(ctxt, &prog.statements, cscript_parsed_statement);

  token* token_it = cscript_vector_begin(tokens, token);
  token* token_it_end = cscript_vector_end(tokens, token);

  for (; token_it != token_it_end;)
    {
    if (ctxt->number_of_syntax_errors > CSCRIPT_MAX_SYNTAX_ERRORS)
      return prog;
    cscript_parsed_statement stmt = cscript_make_statement(ctxt, &token_it, &token_it_end);
    cscript_vector_push_back(ctxt, &prog.statements, stmt, cscript_parsed_statement);
    }

  return prog;
  }

void cscript_program_destroy(cscript_context* ctxt, cscript_program* p)
  {
  cscript_assert(0);
  }

void cscript_statement_destroy(cscript_context* ctxt, cscript_parsed_statement* e)
  {
  cscript_assert(0);
  }