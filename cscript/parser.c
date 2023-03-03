#include "parser.h"
#include "error.h"
#include "context.h"
#include "visitor.h"

#include <string.h>
#include <stdlib.h>

#define CSCRIPT_MAX_SYNTAX_ERRORS 5

static cscript_vector make_null_vector()
  {
  cscript_vector v;
  v.element_size = 0;
  v.vector_capacity = 0;
  v.vector_ptr = 0;
  v.vector_size = 0;
  return v;
  }

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

static cscript_parsed_expression make_null_expr()
  {
  cscript_parsed_expression e;
  e.column_nr = -1;
  e.line_nr = -1;
  e.filename = make_null_string();
  e.fops = make_null_vector();
  e.operands = make_null_vector();
  return e;
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

static int is_assignment(token* t)
  {
  return t->type >= CSCRIPT_T_ASSIGNMENT && t->type <= CSCRIPT_T_ASSIGNMENT_DIV ? 1 : 0;
  }

static int check_token_available(cscript_context* ctxt, token** token_it, token** token_it_end)
  {
  if (*token_it == *token_it_end)
    {
    cscript_string* fn = NULL;
    if (ctxt->filenames_list.vector_size > 0)
      fn = cscript_vector_back(&ctxt->filenames_list, cscript_string);
    cscript_syntax_error_cstr(ctxt, CSCRIPT_ERROR_NO_TOKENS, last_token.line_nr, last_token.column_nr, fn, "");
    return 0;
    }
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

cscript_parsed_variable make_variable(cscript_context* ctxt, token** token_it, token** token_it_end)
  {
  cscript_parsed_variable var;
  var.column_nr = (*token_it)->column_nr;
  var.line_nr = (*token_it)->line_nr;
  var.filename = make_null_string(); 
  var.dims = make_null_vector();
  var.dereference = 0;
  if ((*token_it)->type == CSCRIPT_T_MUL)
    {
    var.dereference = 1;
    if (token_next(ctxt, token_it, token_it_end) == 0)
      return var;
    }
  cscript_string_copy(ctxt, &var.name, &(*token_it)->value);
  if (token_next(ctxt, token_it, token_it_end) == 0)
    return var;
  if ((*token_it)->type == CSCRIPT_T_LEFT_SQUARE_BRACKET) // array call
    {
    cscript_vector_init(ctxt, &var.dims, cscript_parsed_expression);
    token_next(ctxt, token_it, token_it_end);
    cscript_parsed_expression expr = cscript_make_expression(ctxt, token_it, token_it_end);
    cscript_vector_push_back(ctxt, &var.dims, expr, cscript_parsed_expression);
    token_require(ctxt, token_it, token_it_end, "]");
    }
  return var;
  }

cscript_parsed_lvalue_operator make_lvalue_operator(cscript_context* ctxt, token** token_it, token** token_it_end)
  {
  cscript_parsed_lvalue_operator lv;
  lv.column_nr = (*token_it)->column_nr;
  lv.line_nr = (*token_it)->line_nr;
  lv.filename = make_null_string();
  cscript_string_copy(ctxt, &lv.name, &(*token_it)->value);
  token_next(ctxt, token_it, token_it_end);
  lv.lvalue = make_variable(ctxt, token_it, token_it_end);
  return lv;
  }

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
    case CSCRIPT_T_MUL:
      token_next(ctxt, token_it, token_it_end);
      expr.factor.var = make_variable(ctxt, token_it, token_it_end);
      expr.factor.var.dereference = 1;
      expr.type = cscript_factor_type_variable;
      break;
    case CSCRIPT_T_ID:
    {
    token* t = *token_it;
    ++t;
    if (t != *token_it_end && t->type == CSCRIPT_T_LEFT_ROUND_BRACKET)
      {
      cscript_assert(0); //not implemented, this should be a function call
      }
    else
      {
      expr.factor.var = make_variable(ctxt, token_it, token_it_end);
      expr.type = cscript_factor_type_variable;      
      }
    break;
    }
    case CSCRIPT_T_INCREMENT:
    case CSCRIPT_T_DECREMENT:
    {
    expr.factor.lvop = make_lvalue_operator(ctxt, token_it, token_it_end);
    expr.type = cscript_factor_type_lvalue_operator;
    break;
    }
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

cscript_statement make_int(cscript_context* ctxt, token** token_it, token** token_it_end)
  {
  cscript_comma_separated_statements stmts;
  cscript_vector_init(ctxt, &stmts.statements, cscript_statement);
  int done = 0;
  int first_time = 1;
  while (done == 0)
    {
    cscript_parsed_fixnum i;
    i.line_nr = (*token_it)->line_nr;
    i.column_nr = (*token_it)->column_nr;
    i.filename = make_null_string();
    i.expr = make_null_expr();
    cscript_vector_init(ctxt, &i.dims, cscript_parsed_expression);
    if (first_time)
      token_require(ctxt, token_it, token_it_end, "int");

    if (check_token_available(ctxt, token_it, token_it_end))
      cscript_string_copy(ctxt, &i.name, &(*token_it)->value);

    token_next(ctxt, token_it, token_it_end);

    if (current_token_type(token_it, token_it_end) == CSCRIPT_T_LEFT_SQUARE_BRACKET)
      {
      token_next(ctxt, token_it, token_it_end);
      cscript_parsed_expression expr = cscript_make_expression(ctxt, token_it, token_it_end);
      cscript_vector_push_back(ctxt, &i.dims, expr, cscript_parsed_expression);
      token_require(ctxt, token_it, token_it_end, "]");
      }
    if (current_token_type(token_it, token_it_end) == CSCRIPT_T_LEFT_SQUARE_BRACKET)
      {
      cscript_string* fn = NULL;
      if (ctxt->filenames_list.vector_size > 0)
        fn = cscript_vector_back(&ctxt->filenames_list, cscript_string);
      cscript_syntax_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, (*token_it)->line_nr, (*token_it)->column_nr, fn, "only single dimension arrays are allowed");
      break;
      }
    if (current_token_type(token_it, token_it_end) == CSCRIPT_T_ASSIGNMENT)
      {
      token_next(ctxt, token_it, token_it_end);
      i.expr = cscript_make_expression(ctxt, token_it, token_it_end);
      }
    cscript_statement stmt;
    stmt.type = cscript_statement_type_fixnum;
    stmt.statement.fixnum = i;
    cscript_vector_push_back(ctxt, &stmts.statements, stmt, cscript_statement);
    if (current_token_type(token_it, token_it_end) == CSCRIPT_T_COMMA)
      {
      token_require(ctxt, token_it, token_it_end, ",");
      }
    else
      {
      done = 1;
      }
    first_time = 0;
    }
  cscript_statement outstmt;
  outstmt.type = cscript_statement_type_comma_separated;
  outstmt.statement.stmts = stmts;
  return outstmt;
  }


cscript_statement make_float(cscript_context* ctxt, token** token_it, token** token_it_end)
  {
  cscript_comma_separated_statements stmts;
  cscript_vector_init(ctxt, &stmts.statements, cscript_statement);
  int done = 0;
  int first_time = 1;
  while (done == 0)
    {
    cscript_parsed_flonum f;
    f.line_nr = (*token_it)->line_nr;
    f.column_nr = (*token_it)->column_nr;
    f.filename = make_null_string();
    f.expr = make_null_expr();
    cscript_vector_init(ctxt, &f.dims, cscript_parsed_expression);
    if (first_time)
      token_require(ctxt, token_it, token_it_end, "float");

    if (check_token_available(ctxt, token_it, token_it_end))
      cscript_string_copy(ctxt, &f.name, &(*token_it)->value);

    token_next(ctxt, token_it, token_it_end);

    if (current_token_type(token_it, token_it_end) == CSCRIPT_T_LEFT_SQUARE_BRACKET)
      {
      token_next(ctxt, token_it, token_it_end);
      cscript_parsed_expression expr = cscript_make_expression(ctxt, token_it, token_it_end);
      cscript_vector_push_back(ctxt, &f.dims, expr, cscript_parsed_expression);
      token_require(ctxt, token_it, token_it_end, "]");
      }
    if (current_token_type(token_it, token_it_end) == CSCRIPT_T_LEFT_SQUARE_BRACKET)
      {
      cscript_string* fn = NULL;
      if (ctxt->filenames_list.vector_size > 0)
        fn = cscript_vector_back(&ctxt->filenames_list, cscript_string);
      cscript_syntax_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, (*token_it)->line_nr, (*token_it)->column_nr, fn, "only single dimension arrays are allowed");
      break;
      }
    if (current_token_type(token_it, token_it_end) == CSCRIPT_T_ASSIGNMENT)
      {
      token_next(ctxt, token_it, token_it_end);
      f.expr = cscript_make_expression(ctxt, token_it, token_it_end);
      }
    cscript_statement stmt;
    stmt.type = cscript_statement_type_flonum;
    stmt.statement.flonum = f;
    cscript_vector_push_back(ctxt, &stmts.statements, stmt, cscript_statement);
    if (current_token_type(token_it, token_it_end) == CSCRIPT_T_COMMA)
      {
      token_require(ctxt, token_it, token_it_end, ",");
      }
    else
      {
      done = 1;
      }
    first_time = 0;
    }
  cscript_statement outstmt;
  outstmt.type = cscript_statement_type_comma_separated;
  outstmt.statement.stmts = stmts;
  return outstmt;
  }

cscript_statement make_assignment(cscript_context* ctxt, token** token_it, token** token_it_end)
  {
  cscript_parsed_assignment a;
  a.line_nr = (*token_it)->line_nr;
  a.column_nr = (*token_it)->column_nr;
  a.derefence = current_token_type(token_it, token_it_end) == CSCRIPT_T_MUL ? 1 : 0;
  a.name = make_null_string();
  a.filename = make_null_string();
  a.dims = make_null_vector();
  a.op = make_null_string();
  a.expr = make_null_expr();
  cscript_statement outstmt;
  outstmt.type = cscript_statement_type_assignment;
  if (a.derefence)
    {
    if (token_next(ctxt, token_it, token_it_end) == 0)
      {
      outstmt.statement.assignment = a;
      return outstmt;
      }
    }
  cscript_string_copy(ctxt, &a.name, &(*token_it)->value);
  if (token_next(ctxt, token_it, token_it_end) == 0)
    {
    outstmt.statement.assignment = a;
    return outstmt;
    }
  if (current_token_type(token_it, token_it_end) == CSCRIPT_T_LEFT_SQUARE_BRACKET)
    {
    if (a.derefence)
      {
      cscript_string* fn = NULL;
      if (ctxt->filenames_list.vector_size > 0)
        fn = cscript_vector_back(&ctxt->filenames_list, cscript_string);
      cscript_syntax_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, last_token.line_nr, last_token.column_nr, fn, "cannot dereference array");
      outstmt.statement.assignment = a;
      return outstmt;
      }
    token_next(ctxt, token_it, token_it_end);
    cscript_vector_init(ctxt, &a.dims, cscript_parsed_expression);
    cscript_parsed_expression dims_expr = cscript_make_expression(ctxt, token_it, token_it_end);
    cscript_vector_push_back(ctxt, &a.dims, dims_expr, cscript_parsed_expression);
    token_require(ctxt, token_it, token_it_end, "]");
    }
  if (current_token_type(token_it, token_it_end) == CSCRIPT_T_LEFT_SQUARE_BRACKET)
    {
    cscript_string* fn = NULL;
    if (ctxt->filenames_list.vector_size > 0)
      fn = cscript_vector_back(&ctxt->filenames_list, cscript_string);
    cscript_syntax_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, (*token_it)->line_nr, (*token_it)->column_nr, fn, "only single dimension arrays are allowed");
    outstmt.statement.assignment = a;
    return outstmt;
    }
  if (check_token_available(ctxt, token_it, token_it_end) == 0)
    {
    outstmt.statement.assignment = a;
    return outstmt;
    }
  cscript_string_copy(ctxt, &a.op, &(*token_it)->value);
  token_next(ctxt, token_it, token_it_end);
  a.expr = cscript_make_expression(ctxt, token_it, token_it_end);
  outstmt.statement.assignment = a;
  return outstmt;
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

cscript_vector make_parameters(cscript_context* ctxt, token** token_it, token** token_it_end)
  {
  cscript_vector pars = make_null_vector();
  if (*token_it == *token_it_end)
    return pars;
  if (current_token_type(token_it, token_it_end) == CSCRIPT_T_LEFT_ROUND_BRACKET)
    {
    token* t = *token_it;
    ++t;
    if (t != *token_it_end && t->type == CSCRIPT_T_RIGHT_ROUND_BRACKET)
      {
      token_next(ctxt, token_it, token_it_end);
      token_next(ctxt, token_it, token_it_end);
      return pars;
      }
    if (t != *token_it_end && t->type != CSCRIPT_T_ID)
      return pars; // this is an expression probably
    if (token_next(ctxt, token_it, token_it_end)==0)
      return pars;    
    int read_parameters = 1;
    int first_time = 1;
    while (read_parameters)
      {
      if (current_token_type(token_it, token_it_end) == CSCRIPT_T_RIGHT_ROUND_BRACKET)
        {
        break;
        }
      if (current_token_type(token_it, token_it_end) != CSCRIPT_T_ID)
        {
        cscript_string* fn = NULL;
        if (ctxt->filenames_list.vector_size > 0)
          fn = cscript_vector_back(&ctxt->filenames_list, cscript_string);
        cscript_syntax_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, (*token_it)->line_nr, (*token_it)->column_nr, fn, "invalid parameter declaration");
        return pars;
        }
      cscript_parameter p;
      p.line_nr = (*token_it)->line_nr;
      p.column_nr = (*token_it)->column_nr;
      p.filename = make_null_string();
      if (strcmp((*token_it)->value.string_ptr, "int")==0)
        {
        token_require(ctxt, token_it, token_it_end, "int");
        p.type = cscript_parameter_type_fixnum;
        if (current_token_type(token_it, token_it_end) == CSCRIPT_T_MUL)
          {
          p.type = cscript_parameter_type_fixnum_pointer;
          token_next(ctxt, token_it, token_it_end);
          }
        }
      else if (strcmp((*token_it)->value.string_ptr, "float") == 0)
        {
        token_require(ctxt, token_it, token_it_end, "float");
        p.type = cscript_parameter_type_flonum;
        if (current_token_type(token_it, token_it_end) == CSCRIPT_T_MUL)
          {
          p.type = cscript_parameter_type_flonum_pointer;
          token_next(ctxt, token_it, token_it_end);
          }
        }
      else
        {
        cscript_string* fn = NULL;
        if (ctxt->filenames_list.vector_size > 0)
          fn = cscript_vector_back(&ctxt->filenames_list, cscript_string);
        cscript_syntax_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, (*token_it)->line_nr, (*token_it)->column_nr, fn, "invalid parameter declaration");
        return pars;
        }
      if (current_token_type(token_it, token_it_end) != CSCRIPT_T_ID)
        {
        cscript_string* fn = NULL;
        if (ctxt->filenames_list.vector_size > 0)
          fn = cscript_vector_back(&ctxt->filenames_list, cscript_string);
        cscript_syntax_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, (*token_it)->line_nr, (*token_it)->column_nr, fn, "invalid parameter name");
        return pars;
        }
      cscript_string_copy(ctxt, &p.name, &(*token_it)->value);
      token_next(ctxt, token_it, token_it_end);
      if (current_token_type(token_it, token_it_end) == CSCRIPT_T_RIGHT_ROUND_BRACKET)
        {
        read_parameters = 0;
        }
      else
        {
        token_require(ctxt, token_it, token_it_end, ",");
        }
      if (first_time != 0)
        {
        cscript_vector_init(ctxt, &pars, cscript_parameter);
        }
      cscript_vector_push_back(ctxt, &pars, p, cscript_parameter);
      first_time = 0;
      }
    token_require(ctxt, token_it, token_it_end, ")");
    }
  return pars;
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
    case CSCRIPT_T_RIGHT_ROUND_BRACKET:
    {
    cscript_statement nop = make_nop();
    nop.statement.nop.line_nr = (*token_it)->line_nr;
    nop.statement.nop.column_nr = (*token_it)->column_nr;
    return nop;
    }
    case CSCRIPT_T_ID:
    {
    if (strcmp((*token_it)->value.string_ptr, "int") == 0)
      {
      cscript_statement stmt = make_int(ctxt, token_it, token_it_end);
      return stmt;
      }
    if (strcmp((*token_it)->value.string_ptr, "float") == 0)
      {
      cscript_statement stmt = make_float(ctxt, token_it, token_it_end);
      return stmt;
      }
    uint64_t dist = *token_it_end - *token_it;
    if (dist >= 3)
      {
      token* t = *token_it;
      ++t;
      if (is_assignment(t))
        {
        cscript_statement stmt = make_assignment(ctxt, token_it, token_it_end);
        return stmt;
        }
      else if (t->type == CSCRIPT_T_LEFT_SQUARE_BRACKET)
        {
        // find corresponding right square bracket
        int left_squares_encountered = 1;
        int right_squares_encountered = 0;
        token* it = *token_it;
        token* it_end = *token_it_end;
        it += 2;
        while (it != it_end && (left_squares_encountered > right_squares_encountered))
          {
          if (it->type == CSCRIPT_T_LEFT_SQUARE_BRACKET)
            ++left_squares_encountered;
          if (it->type == CSCRIPT_T_RIGHT_SQUARE_BRACKET)
            ++right_squares_encountered;
          ++it;
          }
        if (left_squares_encountered != right_squares_encountered)
          {
          cscript_string* fn = NULL;
          if (ctxt->filenames_list.vector_size > 0)
            fn = cscript_vector_back(&ctxt->filenames_list, cscript_string);
          cscript_syntax_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, last_token.line_nr, last_token.column_nr, fn, "[,] mismatch");
          return make_nop();
          }
        if (it != it_end && is_assignment(it))
          {
          cscript_statement stmt = make_assignment(ctxt, token_it, token_it_end);
          return stmt;
          }
        else
          {
          cscript_statement expr;
          expr.type = cscript_statement_type_expression;
          expr.statement.expr = cscript_make_expression(ctxt, token_it, token_it_end);
          return expr;
          }
        }
      else
        {
        cscript_statement expr;
        expr.type = cscript_statement_type_expression;
        expr.statement.expr = cscript_make_expression(ctxt, token_it, token_it_end);
        return expr;
        }
      }
    else
      {
      cscript_statement expr;
      expr.type = cscript_statement_type_expression;
      expr.statement.expr = cscript_make_expression(ctxt, token_it, token_it_end);
      return expr;
      }
    }
    case CSCRIPT_T_MUL:
    {
    uint64_t dist = *token_it_end - *token_it;
    if (dist >= 4)
      {
      token* t = *token_it;
      t+=2;
      if (is_assignment(t))
        {
        cscript_statement stmt = make_assignment(ctxt, token_it, token_it_end);
        return stmt;
        }
      }
    cscript_statement expr;
    expr.type = cscript_statement_type_expression;
    expr.statement.expr = cscript_make_expression(ctxt, token_it, token_it_end);
    return expr;
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

  prog.parameters = make_parameters(ctxt, &token_it, &token_it_end);

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
static void visit_parameter(cscript_context* ctxt, cscript_visitor* v, cscript_parameter* p)
  {
  UNUSED(v);
  cscript_string_destroy(ctxt, &p->filename);
  cscript_string_destroy(ctxt, &p->name);
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
static void postvisit_statements(cscript_context* ctxt, cscript_visitor* v, cscript_comma_separated_statements* e)
  {
  UNUSED(v);
  cscript_vector_destroy(ctxt, &e->statements);
  }
static void postvisit_fixnum(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_fixnum* fx)
  {
  UNUSED(v);
  cscript_vector_destroy(ctxt, &fx->dims);
  cscript_string_destroy(ctxt, &fx->filename);
  cscript_string_destroy(ctxt, &fx->name);
  }
static void postvisit_assignment(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_assignment* a)
  {
  UNUSED(v);
  cscript_vector_destroy(ctxt, &a->dims);
  cscript_string_destroy(ctxt, &a->filename);
  cscript_string_destroy(ctxt, &a->name);
  cscript_string_destroy(ctxt, &a->op);
  }
static void postvisit_lvalueop(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_lvalue_operator* l)
  {
  UNUSED(v);
  cscript_string_destroy(ctxt, &l->filename);
  cscript_string_destroy(ctxt, &l->name);
  }
static void postvisit_for(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_for* f)
  {
  UNUSED(v);
  cscript_string_destroy(ctxt, &f->filename);
  cscript_vector_destroy(ctxt, &f->init_cond_inc);
  cscript_vector_destroy(ctxt, &f->statements);
  }
static void postvisit_if(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_if* i)
  {
  UNUSED(v);
  cscript_string_destroy(ctxt, &i->filename);
  cscript_vector_destroy(ctxt, &i->condition);
  cscript_vector_destroy(ctxt, &i->body);
  cscript_vector_destroy(ctxt, &i->alternative);
  }
static void postvisit_variable(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_variable* var)
  {
  UNUSED(v);
  cscript_string_destroy(ctxt, &var->filename);
  cscript_string_destroy(ctxt, &var->name);
  cscript_vector_destroy(ctxt, &var->dims);
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
  destroyer.visitor->postvisit_statements = postvisit_statements;
  destroyer.visitor->postvisit_fixnum = postvisit_fixnum;
  destroyer.visitor->postvisit_var = postvisit_variable;
  destroyer.visitor->postvisit_assignment = postvisit_assignment;
  destroyer.visitor->postvisit_lvalueop = postvisit_lvalueop;
  destroyer.visitor->postvisit_for = postvisit_for;
  destroyer.visitor->postvisit_if = postvisit_if;
  destroyer.visitor->visit_parameter = visit_parameter;
  cscript_visit_program(ctxt, destroyer.visitor, p);

  cscript_vector_destroy(ctxt, &p->parameters);
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