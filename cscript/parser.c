#include "parser.h"
#include "error.h"
#include "context.h"
#include "visitor.h"

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

static int token_next(cscript_context* ctxt, token** token_it, token** token_it_end)
  {
  if (*token_it == *token_it_end)
    {
    cscript_string* fn = NULL;
    if (ctxt->filenames_list.vector_size > 0)
      fn = cscript_vector_back(&ctxt->filenames_list, cscript_string);
    cscript_syntax_error_cstr(ctxt, CSCRIPT_ERROR_NO_TOKENS, last_token.line_nr, last_token.column_nr, fn, "");
    return 0;
    }
  popped_token = (**token_it);
  last_token = (**token_it);
  ++(*token_it);
  return 1;
  }

static int token_require(cscript_context* ctxt, token** token_it, token** token_it_end, const char* required)
  {
  if (*token_it == *token_it_end)
    {
    cscript_string* fn = NULL;
    if (ctxt->filenames_list.vector_size > 0)
      fn = cscript_vector_back(&ctxt->filenames_list, cscript_string);
    cscript_syntax_error_cstr(ctxt, CSCRIPT_ERROR_NO_TOKENS, last_token.line_nr, last_token.column_nr, fn, "");
    return 0;
    }
  else
    {
    if (strcmp((*token_it)->value.string_ptr, required) != 0)
      {
      cscript_string* fn = NULL;
      if (ctxt->filenames_list.vector_size > 0)
        fn = cscript_vector_back(&ctxt->filenames_list, cscript_string);
      cscript_syntax_error_cstr(ctxt, CSCRIPT_ERROR_EXPECTED_KEYWORD, (*token_it)->line_nr, (*token_it)->column_nr, fn, required);
      return 0;
      }
    popped_token = (**token_it);
    last_token = (**token_it);
    ++(*token_it);
    return 1;
    }
  }

static int next_token_equals(token** token_it, token** token_it_end, const char* expected)
  {
  if (*token_it == *token_it_end)
    return 0;
  token* it = *token_it;
  ++it;
  if (it == *token_it_end)
    return 0;
  return strcmp(it->value.string_ptr, expected) == 0;
  }

static int next_token_type(token** token_it, token** token_it_end)
  {
  if (*token_it == *token_it_end)
    return CSCRIPT_T_BAD;
  token* it = *token_it;
  ++it;
  if (it == *token_it_end)
    return CSCRIPT_T_BAD;
  return it->type;
  }

static int current_token_equals(token** token_it, token** token_it_end, const char* expected)
  {
  if (*token_it == *token_it_end)
    return 0;
  if ((*token_it)->type == CSCRIPT_T_FIXNUM || (*token_it)->type == CSCRIPT_T_FLONUM)
    return 0;
  return strcmp((*token_it)->value.string_ptr, expected) == 0;
  }

static int current_token_type(token** token_it, token** token_it_end)
  {
  if (*token_it == *token_it_end)
    return CSCRIPT_T_BAD;
  return (*token_it)->type;
  }

static void check_for_semicolon(cscript_context* ctxt, token** token_it, token** token_it_end, cscript_statement* last_statement)
  {
  int optional = 0;
  if (last_statement->type == cscript_statement_type_if || last_statement->type == cscript_statement_type_for)
    optional = 1;
  if (optional)
    {
    if (current_token_equals(token_it, token_it_end, ";"))
      token_next(ctxt, token_it, token_it_end);
    }
  else
    {
    token_require(ctxt, token_it, token_it_end, ";");
    }
  }

static cscript_statement make_nop()
  {
  cscript_parsed_nop nop;
  nop.column_nr = -1;
  nop.line_nr = -1;
  nop.filename = make_null_string();
  cscript_statement stmt;
  stmt.type = cscript_statement_type_nop;
  stmt.statement.nop = nop;
  return stmt;
  }

cscript_parsed_expression cscript_make_expression(cscript_context* ctxt, token** token_it, token** token_it_end);

cscript_parsed_factor cscript_make_factor(cscript_context* ctxt, token** token_it, token** token_it_end)
  {
  cscript_parsed_factor expr;  
  expr.sign = '+';
  int line_nr = (*token_it)->line_nr;
  int column_nr = (*token_it)->column_nr;
  if (current_token_equals(token_it, token_it_end, "+"))
    {
    expr.sign = '+';
    token_next(ctxt, token_it, token_it_end);
    }
  else if (current_token_equals(token_it, token_it_end, "-"))
    {
    expr.sign = '-';
    token_next(ctxt, token_it, token_it_end);
    }
  if (*token_it == *token_it_end)
    {
    cscript_string* fn = NULL;
    if (ctxt->filenames_list.vector_size > 0)
      fn = cscript_vector_back(&ctxt->filenames_list, cscript_string);
    cscript_syntax_error_cstr(ctxt, CSCRIPT_ERROR_NO_TOKENS, last_token.line_nr, last_token.column_nr, fn, "");
    expr.type = cscript_factor_type_number;
    expr.factor.number.number.fl = 0;
    expr.factor.number.type = cscript_number_type_fixnum;
    return expr;
    }
  switch (current_token_type(token_it, token_it_end))
    {
    case CSCRIPT_T_LEFT_ROUND_BRACKET:
      token_next(ctxt, token_it, token_it_end);
      expr.factor.expr = cscript_make_expression(ctxt, token_it, token_it_end);
      expr.type = cscript_factor_type_expression;
      token_require(ctxt, token_it, token_it_end, ")");
      break;
    case CSCRIPT_T_FLONUM:
      expr.factor.number.column_nr = column_nr;
      expr.factor.number.line_nr = line_nr;
      expr.factor.number.type = cscript_number_type_flonum;
      expr.factor.number.number.fl = cscript_to_flonum((*token_it)->value.string_ptr);
      expr.factor.number.filename = make_null_string();
      expr.type = cscript_factor_type_number;
      token_next(ctxt, token_it, token_it_end);
      break;
    case CSCRIPT_T_FIXNUM:
      expr.factor.number.column_nr = column_nr;
      expr.factor.number.line_nr = line_nr;
      expr.factor.number.type = cscript_number_type_fixnum;
      expr.factor.number.number.fx = cscript_to_fixnum((*token_it)->value.string_ptr);
      expr.factor.number.filename = make_null_string();
      expr.type = cscript_factor_type_number;
      token_next(ctxt, token_it, token_it_end);
      break;
    default:
      cscript_assert(0); //not implemented
    }
  return expr;
  }

cscript_parsed_term cscript_make_term(cscript_context* ctxt, token** token_it, token** token_it_end)
  {
  cscript_parsed_term expr;
  expr.filename = make_null_string();
  expr.line_nr = (*token_it)->line_nr;
  expr.column_nr = (*token_it)->column_nr;
  cscript_vector_init(ctxt, &expr.operands, cscript_parsed_factor);
  cscript_vector_init(ctxt, &expr.fops, int);
  cscript_parsed_factor f1 = cscript_make_factor(ctxt, token_it, token_it_end);
  cscript_vector_push_back(ctxt, &expr.operands, f1, cscript_parsed_factor);
  while (1)
    {
    if (token_it == token_it_end)
      break;
    if ((*token_it)->value.string_length != 1)
      break;
    char op = (*token_it)->value.string_ptr[0];
    switch (op)
      {
      case '*':
      {
      cscript_vector_push_back(ctxt, &expr.fops, cscript_op_mul, int);
      token_next(ctxt, token_it, token_it_end);
      cscript_parsed_factor f2 = cscript_make_factor(ctxt, token_it, token_it_end);
      cscript_vector_push_back(ctxt, &expr.operands, f2, cscript_parsed_factor);
      continue;
      }
      case '/':
      {
      cscript_vector_push_back(ctxt, &expr.fops, cscript_op_div, int);
      token_next(ctxt, token_it, token_it_end);
      cscript_parsed_factor f2 = cscript_make_factor(ctxt, token_it, token_it_end);
      cscript_vector_push_back(ctxt, &expr.operands, f2, cscript_parsed_factor);
      continue;
      }
      case '%':
      {
      cscript_vector_push_back(ctxt, &expr.fops, cscript_op_percent, int);
      token_next(ctxt, token_it, token_it_end);
      cscript_parsed_factor f2 = cscript_make_factor(ctxt, token_it, token_it_end);
      cscript_vector_push_back(ctxt, &expr.operands, f2, cscript_parsed_factor);
      continue;
      }
      default:
        break;
      }
    break;
    }
  return expr;
  }

cscript_parsed_relop cscript_make_relop(cscript_context* ctxt, token** token_it, token** token_it_end)
  {
  cscript_parsed_relop expr;
  expr.filename = make_null_string();
  expr.line_nr = (*token_it)->line_nr;
  expr.column_nr = (*token_it)->column_nr;
  cscript_vector_init(ctxt, &expr.operands, cscript_parsed_term);
  cscript_vector_init(ctxt, &expr.fops, int);
  cscript_parsed_term t1 = cscript_make_term(ctxt, token_it, token_it_end);
  cscript_vector_push_back(ctxt, &expr.operands, t1, cscript_parsed_term);
  while (1)
    {
    if (token_it == token_it_end)
      break;
    if ((*token_it)->value.string_length != 1)
      break;
    char op = (*token_it)->value.string_ptr[0];
    switch (op)
      {
      case '+':
      {
      cscript_vector_push_back(ctxt, &expr.fops, cscript_op_plus, int);
      token_next(ctxt, token_it, token_it_end);
      cscript_parsed_term t2 = cscript_make_term(ctxt, token_it, token_it_end);
      cscript_vector_push_back(ctxt, &expr.operands, t2, cscript_parsed_term);
      continue;
      }
      case '-':
      {
      cscript_vector_push_back(ctxt, &expr.fops, cscript_op_minus, int);
      token_next(ctxt, token_it, token_it_end);
      cscript_parsed_term t2 = cscript_make_term(ctxt, token_it, token_it_end);
      cscript_vector_push_back(ctxt, &expr.operands, t2, cscript_parsed_term);
      continue;
      }
      default:
        break;
      }
    break;
    }
  return expr;
  }

cscript_parsed_expression cscript_make_expression(cscript_context* ctxt, token** token_it, token** token_it_end)
  {
  cscript_parsed_expression expr;
  expr.filename = make_null_string();
  expr.line_nr = (*token_it)->line_nr;
  expr.column_nr = (*token_it)->column_nr;
  cscript_vector_init(ctxt, &expr.operands, cscript_parsed_relop);
  cscript_vector_init(ctxt, &expr.fops, int);
  cscript_parsed_relop r1 = cscript_make_relop(ctxt, token_it, token_it_end);
  cscript_vector_push_back(ctxt, &expr.operands, r1, cscript_parsed_relop);
  while (1)
    {
    if (token_it == token_it_end)
      break;
    const uint32_t len = (*token_it)->value.string_length;
    if (len > 2)
      break;
    char op1 = (*token_it)->value.string_ptr[0];
    char op2 = (*token_it)->value.string_ptr[1];
    switch (op1)
      {
      case '<':
      {
      if (len == 1)
        {
        cscript_vector_push_back(ctxt, &expr.fops, cscript_op_less, int);
        token_next(ctxt, token_it, token_it_end);
        cscript_parsed_relop r2 = cscript_make_relop(ctxt, token_it, token_it_end);
        cscript_vector_push_back(ctxt, &expr.operands, r2, cscript_parsed_relop);
        continue;
        }
      else if (len == 2 && op2 == '=')
        {
        cscript_vector_push_back(ctxt, &expr.fops, cscript_op_leq, int);
        token_next(ctxt, token_it, token_it_end);
        cscript_parsed_relop r2 = cscript_make_relop(ctxt, token_it, token_it_end);
        cscript_vector_push_back(ctxt, &expr.operands, r2, cscript_parsed_relop);
        continue;
        }
      break;
      }
      case '>':
      {
      if (len == 1)
        {
        cscript_vector_push_back(ctxt, &expr.fops, cscript_op_greater, int);
        token_next(ctxt, token_it, token_it_end);
        cscript_parsed_relop r2 = cscript_make_relop(ctxt, token_it, token_it_end);
        cscript_vector_push_back(ctxt, &expr.operands, r2, cscript_parsed_relop);
        continue;
        }
      else if (len == 2 && op2 == '=')
        {
        cscript_vector_push_back(ctxt, &expr.fops, cscript_op_geq, int);
        token_next(ctxt, token_it, token_it_end);
        cscript_parsed_relop r2 = cscript_make_relop(ctxt, token_it, token_it_end);
        cscript_vector_push_back(ctxt, &expr.operands, r2, cscript_parsed_relop);
        continue;
        }
      break;
      }
      case '=':
      {
      if (len == 2 && op2 == '=')
        {
        cscript_vector_push_back(ctxt, &expr.fops, cscript_op_equal, int);
        token_next(ctxt, token_it, token_it_end);
        cscript_parsed_relop r2 = cscript_make_relop(ctxt, token_it, token_it_end);
        cscript_vector_push_back(ctxt, &expr.operands, r2, cscript_parsed_relop);
        continue;
        }
      break;
      }
      case '!':
      {
      if (len == 2 && op2 == '=')
        {
        cscript_vector_push_back(ctxt, &expr.fops, cscript_op_not_equal, int);
        token_next(ctxt, token_it, token_it_end);
        cscript_parsed_relop r2 = cscript_make_relop(ctxt, token_it, token_it_end);
        cscript_vector_push_back(ctxt, &expr.operands, r2, cscript_parsed_relop);
        continue;
        }
      break;
      }
      default:
        break;
      }
    break;
    }
  return expr;
  }

cscript_statement cscript_make_statement(cscript_context* ctxt, token** token_it, token** token_it_end)
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
    case CSCRIPT_T_SEMICOLON:
    {
    cscript_statement nop = make_nop();
    nop.statement.nop.line_nr = (*token_it)->line_nr;
    nop.statement.nop.column_nr = (*token_it)->column_nr;
    return nop;
    }
    default:
    {
    cscript_statement expr;
    expr.type = cscript_statement_type_expression;
    expr.statement.expr = cscript_make_expression(ctxt, token_it, token_it_end);
    return expr;
    }

    }

  return make_nop();
  }

cscript_program make_program(cscript_context* ctxt, cscript_vector* tokens)
  {
  last_token = make_bad_token();
  cscript_syntax_errors_clear(ctxt);
  invalidate_popped();
  cscript_program prog;
  cscript_vector_init(ctxt, &prog.statements, cscript_statement);

  token* token_it = cscript_vector_begin(tokens, token);
  token* token_it_end = cscript_vector_end(tokens, token);

  for (; token_it != token_it_end;)
    {
    if (ctxt->number_of_syntax_errors > CSCRIPT_MAX_SYNTAX_ERRORS)
      return prog;
    cscript_statement stmt = cscript_make_statement(ctxt, &token_it, &token_it_end);
    cscript_vector_push_back(ctxt, &prog.statements, stmt, cscript_statement);
    check_for_semicolon(ctxt, &token_it, &token_it_end, &stmt);
    }

  return prog;
  }


typedef struct cscript_program_destroy_visitor
  {
  cscript_visitor* visitor;
  } cscript_program_destroy_visitor;

static void visit_nop(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_nop* e)
  {
  UNUSED(v);
  cscript_string_destroy(ctxt, &e->filename);
  }
static void visit_number(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_number* e)
  {
  UNUSED(v);
  cscript_string_destroy(ctxt, &e->filename);
  }
static void postvisit_expression(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_expression* e)
  {
  UNUSED(v);
  cscript_string_destroy(ctxt, &e->filename);
  cscript_vector_destroy(ctxt, &e->operands);
  cscript_vector_destroy(ctxt, &e->fops);
  }
static void postvisit_relop(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_relop* e)
  {
  UNUSED(v);
  cscript_string_destroy(ctxt, &e->filename);
  cscript_vector_destroy(ctxt, &e->operands);
  cscript_vector_destroy(ctxt, &e->fops);
  }
static void postvisit_term(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_term* e)
  {
  UNUSED(v);
  cscript_string_destroy(ctxt, &e->filename);
  cscript_vector_destroy(ctxt, &e->operands);
  cscript_vector_destroy(ctxt, &e->fops);
  }

void cscript_program_destroy(cscript_context* ctxt, cscript_program* p)
  {
  cscript_program_destroy_visitor destroyer;
  destroyer.visitor = cscript_visitor_new(ctxt, &destroyer);

  destroyer.visitor->visit_nop = visit_nop;
  destroyer.visitor->visit_number = visit_number;
  destroyer.visitor->postvisit_term = postvisit_term;
  destroyer.visitor->postvisit_relop = postvisit_relop;
  destroyer.visitor->postvisit_expression = postvisit_expression;

  cscript_visit_program(ctxt, destroyer.visitor, p);

  cscript_vector_destroy(ctxt, &p->statements);

  destroyer.visitor->destroy(ctxt, destroyer.visitor);
  }

void cscript_statement_destroy(cscript_context* ctxt, cscript_statement* e)
  {
  cscript_program_destroy_visitor destroyer;
  destroyer.visitor = cscript_visitor_new(ctxt, &destroyer);

  destroyer.visitor->visit_nop = visit_nop;

  cscript_visit_statement(ctxt, destroyer.visitor, e);

  destroyer.visitor->destroy(ctxt, destroyer.visitor);
  }