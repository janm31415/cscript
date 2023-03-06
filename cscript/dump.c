#include "dump.h"
#include "syscalls.h"
#include <string.h>

static void dump_expression(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_expression* e);
static void dump_statement(cscript_context* ctxt, cscript_visitor* v, cscript_statement* s);

static int dump_var(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_variable* e)
  {
  cscript_dump_visitor* d = (cscript_dump_visitor*)(v->impl);
  if (e->dereference != 0)
    cscript_string_append_cstr(ctxt, &(d->s), "*"); 
  cscript_string_append_cstr(ctxt, &(d->s), e->name.string_ptr);
  if (e->dims.vector_size > 0)
    {
    cscript_string_append_cstr(ctxt, &(d->s), "[");
    cscript_parsed_expression* ex = cscript_vector_begin(&e->dims, cscript_parsed_expression);
    dump_expression(ctxt, v, ex);
    cscript_string_append_cstr(ctxt, &(d->s), "]");
    }
  return 1;
  }

static void append_op(cscript_context* ctxt, cscript_string* s, int op)
  {
  switch (op)
    {
    case cscript_op_mul:
      cscript_string_append_cstr(ctxt, s, "*");
      break;
    case cscript_op_div:
      cscript_string_append_cstr(ctxt, s, "/");
      break;
    case cscript_op_percent:
      cscript_string_append_cstr(ctxt, s, "%");
      break;
    case cscript_op_plus:
      cscript_string_append_cstr(ctxt, s, "+");
      break;
    case cscript_op_minus:
      cscript_string_append_cstr(ctxt, s, "-");
      break;
    case cscript_op_less:
      cscript_string_append_cstr(ctxt, s, "<");
      break;
    case cscript_op_leq:
      cscript_string_append_cstr(ctxt, s, "<=");
      break;
    case cscript_op_greater:
      cscript_string_append_cstr(ctxt, s, ">");
      break;
    case cscript_op_geq:
      cscript_string_append_cstr(ctxt, s, ">=");
      break;
    case cscript_op_equal:
      cscript_string_append_cstr(ctxt, s, "==");
      break;
    case cscript_op_not_equal:
      cscript_string_append_cstr(ctxt, s, "!=");
      break;
    }
  }

static void dump_number(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_number* e)
  {
  cscript_dump_visitor* d = (cscript_dump_visitor*)(v->impl);
  char buffer[80];
  memset(buffer, 0, 80 * sizeof(char));
  switch (e->type)
    {
    case cscript_number_type_fixnum:
      cscript_fixnum_to_char(buffer, e->number.fx);
      cscript_string_append_cstr(ctxt, &(d->s), buffer);
      break;
    case cscript_number_type_flonum:
      cscript_flonum_to_char(buffer, e->number.fl);
      cscript_string_append_cstr(ctxt, &(d->s), buffer);
      break;
    }
  }

static void dump_lvalueop(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_lvalue_operator* l)
  {
  
  }

static void dump_function(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_function* f)
  {
  }

static void dump_expression_list(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_expression_list* l)
  {
  }

static void dump_factor(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_factor* e)
  {
  cscript_dump_visitor* d = (cscript_dump_visitor*)(v->impl);
  switch (e->type)
    {
    case cscript_factor_type_number:
    {
    dump_number(ctxt, v, &e->factor.number);
    break;
    }
    case cscript_factor_type_expression:
    {
    cscript_string_append_cstr(ctxt, &(d->s), "(");
    dump_expression(ctxt, v, &e->factor.expr);
    cscript_string_append_cstr(ctxt, &(d->s), ")");
    break;
    }
    case cscript_factor_type_variable:
    {
    dump_var(ctxt, v, &e->factor.var);
    break;
    }
    case cscript_factor_type_lvalue_operator:
    {
    dump_lvalueop(ctxt, v, &e->factor.lvop);
    break;
    }
    case cscript_factor_type_function:
    {
    dump_function(ctxt, v, &e->factor.fun);
    break;
    }
    case cscript_factor_type_expression_list:
    {
    dump_expression_list(ctxt, v, &e->factor.exprlist);
    break;
    }
    }
  }

static void dump_expression(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_expression* e)
  {
  cscript_dump_visitor* d = (cscript_dump_visitor*)(v->impl);
  cscript_vector* operands = &e->operands;
  cscript_parsed_relop* expr_it = cscript_vector_begin(operands, cscript_parsed_relop);
  cscript_parsed_relop* expr_it_end = cscript_vector_end(operands, cscript_parsed_relop);
  int* expr_op = cscript_vector_begin(&e->fops, int);
  int* expr_op_end = cscript_vector_end(&e->fops, int);
  for (; expr_it != expr_it_end; ++expr_it)
    {
    cscript_parsed_term* relop_it = cscript_vector_begin(&expr_it->operands, cscript_parsed_term);
    cscript_parsed_term* relop_it_end = cscript_vector_end(&expr_it->operands, cscript_parsed_term);
    int* relop_op = cscript_vector_begin(&expr_it->fops, int);
    int* relop_op_end = cscript_vector_end(&expr_it->fops, int);
    for (; relop_it != relop_it_end; ++relop_it)
      {

      cscript_parsed_factor* term_it = cscript_vector_begin(&relop_it->operands, cscript_parsed_factor);
      cscript_parsed_factor* term_it_end = cscript_vector_end(&relop_it->operands, cscript_parsed_factor);
      int* term_op = cscript_vector_begin(&relop_it->fops, int);
      int* term_op_end = cscript_vector_end(&relop_it->fops, int);
      for (; term_it != term_it_end; ++term_it)
        {
        dump_factor(ctxt, v, term_it);
        if (term_op != term_op_end)
          {
          append_op(ctxt, &(d->s), *term_op);
          ++term_op;
          }
        }

      if (relop_op != relop_op_end)
        {
        append_op(ctxt, &(d->s), *relop_op);
        ++relop_op;
        }
      }

    if (expr_op != expr_op_end)
      {
      append_op(ctxt, &(d->s), *expr_op);
      ++expr_op;
      }
    }
  }

static void dump_fixnum(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_fixnum* fx)
  {
  cscript_dump_visitor* d = (cscript_dump_visitor*)(v->impl);
  cscript_string_append_cstr(ctxt, &(d->s), "int ");
  cscript_string_append_cstr(ctxt, &(d->s), fx->name.string_ptr);
  if (fx->dims.vector_size > 0)
    {
    cscript_string_append_cstr(ctxt, &(d->s), "[");
    cscript_parsed_expression* e = cscript_vector_begin(&fx->dims, cscript_parsed_expression);
    dump_expression(ctxt, v, e);
    cscript_string_append_cstr(ctxt, &(d->s), "]");
    }
  if (fx->expr.operands.vector_size > 0)
    {
    cscript_string_append_cstr(ctxt, &(d->s), " = ");
    dump_expression(ctxt, v, &fx->expr);
    }
  }

static void dump_flonum(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_flonum* fl)
  {
  cscript_dump_visitor* d = (cscript_dump_visitor*)(v->impl);
  cscript_string_append_cstr(ctxt, &(d->s), "float ");
  cscript_string_append_cstr(ctxt, &(d->s), fl->name.string_ptr);
  if (fl->dims.vector_size > 0)
    {
    cscript_string_append_cstr(ctxt, &(d->s), "[");
    cscript_parsed_expression* e = cscript_vector_begin(&fl->dims, cscript_parsed_expression);
    dump_expression(ctxt, v, e);
    cscript_string_append_cstr(ctxt, &(d->s), "]");
    }
  if (fl->expr.operands.vector_size > 0)
    {
    cscript_string_append_cstr(ctxt, &(d->s), " = ");
    dump_expression(ctxt, v, &fl->expr);
    }
  }

static void dump_assignment(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_assignment* a)
  {
  cscript_dump_visitor* d = (cscript_dump_visitor*)(v->impl);
  if (a->derefence != 0)
    cscript_string_append_cstr(ctxt, &(d->s), "*");
  cscript_string_append_cstr(ctxt, &(d->s), a->name.string_ptr);
  if (a->dims.vector_size > 0)
    {
    cscript_string_append_cstr(ctxt, &(d->s), "[");
    cscript_parsed_expression* e = cscript_vector_begin(&a->dims, cscript_parsed_expression);
    dump_expression(ctxt, v, e);
    cscript_string_append_cstr(ctxt, &(d->s), "]");
    }
  cscript_string_append_cstr(ctxt, &(d->s), " ");
  cscript_string_append_cstr(ctxt, &(d->s), a->op.string_ptr);
  cscript_string_append_cstr(ctxt, &(d->s), " ");
  dump_expression(ctxt, v, &a->expr);    
  }

static void dump_comma_separated_statements(cscript_context* ctxt, cscript_visitor* v, cscript_comma_separated_statements* s)
  {
  cscript_dump_visitor* d = (cscript_dump_visitor*)(v->impl);
  cscript_statement* it = cscript_vector_begin(&s->statements, cscript_statement);
  cscript_statement* it_end = cscript_vector_end(&s->statements, cscript_statement);
  for (; it != it_end; ++it)
    {
    dump_statement(ctxt, v, it);
    if (it+1 != it_end)
      {
      cscript_string_append_cstr(ctxt, &(d->s), "; ");
      }
    }
  }


static void dump_statement(cscript_context* ctxt, cscript_visitor* v, cscript_statement* s)
  {
  cscript_dump_visitor* d = (cscript_dump_visitor*)(v->impl);
  switch (s->type)
    {
    case cscript_statement_type_expression:
      dump_expression(ctxt, v, &s->statement.expr);
      break;
    case cscript_statement_type_fixnum:
      dump_fixnum(ctxt, v, &s->statement.fixnum);
      break;
    case cscript_statement_type_flonum:
      dump_flonum(ctxt, v, &s->statement.flonum);
      return 0;
    case cscript_statement_type_for:
      cscript_assert(0); // todo
      break;
    case cscript_statement_type_if:
      cscript_assert(0); // todo
      break;
    case cscript_statement_type_comma_separated:
      dump_comma_separated_statements(ctxt, v, &s->statement.stmts);
      break;
    case cscript_statement_type_nop:
      break;
    case cscript_statement_type_assignment:
      dump_assignment(ctxt, v, &s->statement.assignment);
      break;
    default:
      break;
    }
  }

static int previsit_statement(cscript_context* ctxt, cscript_visitor* v, cscript_statement* s)
  {
  cscript_dump_visitor* d = (cscript_dump_visitor*)(v->impl);
  dump_statement(ctxt, v, s);
  cscript_string_append_cstr(ctxt, &(d->s), ";\n");
  return 0;
  }

cscript_dump_visitor* cscript_dump_visitor_new(cscript_context* ctxt)
  {
  cscript_dump_visitor* v = cscript_new(ctxt, cscript_dump_visitor);
  v->visitor = cscript_visitor_new(ctxt, v);
  cscript_string_init(ctxt, &(v->s), "");
  
  v->visitor->previsit_statement = previsit_statement;
  return v;
  }

void cscript_dump_visitor_free(cscript_context* ctxt, cscript_dump_visitor* v)
  {
  if (v)
    {
    cscript_string_destroy(ctxt, &(v->s));
    v->visitor->destroy(ctxt, v->visitor);
    cscript_delete(ctxt, v);
    }
  }

cscript_string cscript_dump(cscript_context* ctxt, cscript_program* prog)
  {
  cscript_dump_visitor* dumper = cscript_dump_visitor_new(ctxt);
  cscript_visit_program(ctxt, dumper->visitor, prog);
  cscript_string s;
  cscript_string_copy(ctxt, &s, &dumper->s);
  cscript_dump_visitor_free(ctxt, dumper);
  return s;
  }

cscript_string cscript_dump_statement(cscript_context* ctxt, cscript_statement* e)
  {
  cscript_dump_visitor* dumper = cscript_dump_visitor_new(ctxt);
  cscript_visit_statement(ctxt, dumper->visitor, e);
  cscript_string s;
  cscript_string_copy(ctxt, &s, &dumper->s);
  cscript_dump_visitor_free(ctxt, dumper);
  return s;
  }