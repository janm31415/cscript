#include "dump.h"

static void visit_nop(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_nop* e)
  {
  UNUSED(e);
  }

static void visit_var(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_variable* e)
  {
  UNUSED(e);
  cscript_dump_visitor* d = (cscript_dump_visitor*)(v->impl);
  cscript_string_append_cstr(ctxt, &(d->s), e->name.string_ptr);
  }

static void visit_number(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_variable* e)
  {
  UNUSED(e);
  cscript_dump_visitor* d = (cscript_dump_visitor*)(v->impl);
  cscript_string_append_cstr(ctxt, &(d->s), e->name.string_ptr);
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

static int previsit_expression(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_expression* e);

static void dump_number(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_number* e)
  {
  cscript_dump_visitor* d = (cscript_dump_visitor*)(v->impl);
  char buffer[80];
  memset(buffer, 0, 80 * sizeof(char));
  switch (e->type)
    {
    case cscript_number_type_fixnum:
      cscript_fixnum_to_char(buffer, e->number.fl);
      cscript_string_append_cstr(ctxt, &(d->s), buffer);
      break;
    case cscript_number_type_flonum:
      cscript_flonum_to_char(buffer, e->number.fl);
      cscript_string_append_cstr(ctxt, &(d->s), buffer);
      break;
    }
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
    previsit_expression(ctxt, v, &e->factor.expr);
    cscript_string_append_cstr(ctxt, &(d->s), ")");
    break;
    }
    case cscript_factor_type_variable:
    {
    visit_var(ctxt, v, &e->factor.var);
    break;
    }
    }
  }

static int previsit_expression(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_expression* e)
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
  return 0;
  }

void postvisit_statement(cscript_context* ctxt, cscript_visitor* v, cscript_statement* e)
  {
  UNUSED(e);
  cscript_dump_visitor* d = (cscript_dump_visitor*)(v->impl);
  cscript_string_append_cstr(ctxt, &(d->s), ";\n");
  }

cscript_dump_visitor* cscript_dump_visitor_new(cscript_context* ctxt)
  {
  cscript_dump_visitor* v = cscript_new(ctxt, cscript_dump_visitor);
  v->visitor = cscript_visitor_new(ctxt, v);
  cscript_string_init(ctxt, &(v->s), "");
  v->visitor->visit_nop = visit_nop;
  v->visitor->visit_var = visit_var;
  v->visitor->previsit_expression = previsit_expression;
  v->visitor->postvisit_statement = postvisit_statement;
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