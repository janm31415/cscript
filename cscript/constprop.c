#include "constprop.h"
#include "visitor.h"
#include "constant.h"

#include <string.h>

typedef struct cscript_is_mutable_variable_visitor
  {
  cscript_visitor* visitor;
  cscript_map* is_unmutable;
  } cscript_is_mutable_variable_visitor;

static int previsit_fixnum(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_fixnum* fx)
  {
  cscript_is_mutable_variable_visitor* vis = (cscript_is_mutable_variable_visitor*)(v->impl);
  cscript_object key;
  key.type = cscript_object_type_string;
  key.value.s = fx->name;
  cscript_object* value = cscript_map_insert(ctxt, vis->is_unmutable, &key);
  value->type = cscript_object_type_fixnum;
  if (fx->name.string_ptr[0] == '$' || fx->dims.vector_size > 0)
    value->value.fx = 0;
  else
    value->value.fx = 1;
  return 1;
  }

static int previsit_flonum(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_flonum* fl)
  {
  cscript_is_mutable_variable_visitor* vis = (cscript_is_mutable_variable_visitor*)(v->impl);
  cscript_object key;
  key.type = cscript_object_type_string;
  key.value.s = fl->name;
  cscript_object* value = cscript_map_insert(ctxt, vis->is_unmutable, &key);
  value->type = cscript_object_type_fixnum;
  if (fl->name.string_ptr[0] == '$' || fl->dims.vector_size > 0)
    value->value.fx = 0;
  else
    value->value.fx = 1;
  return 1;
  }

static int previsit_assignment(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_assignment* a)
  {
  cscript_is_mutable_variable_visitor* vis = (cscript_is_mutable_variable_visitor*)(v->impl);
  cscript_object key;
  key.type = cscript_object_type_string;
  key.value.s = a->name;
  cscript_object* value = cscript_map_insert(ctxt, vis->is_unmutable, &key);
  value->type = cscript_object_type_fixnum;
  value->value.fx = 0;
  return 1;
  }

static int previsit_lvalueop(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_lvalue_operator* l)
  {
  cscript_is_mutable_variable_visitor* vis = (cscript_is_mutable_variable_visitor*)(v->impl);
  cscript_object key;
  key.type = cscript_object_type_string;
  key.value.s = l->lvalue.name;
  cscript_object* value = cscript_map_insert(ctxt, vis->is_unmutable, &key);
  value->type = cscript_object_type_fixnum;
  value->value.fx = 0;
  return 1;
  }

static cscript_is_mutable_variable_visitor* cscript_is_mutable_variable_visitor_new(cscript_context* ctxt)
  {
  cscript_is_mutable_variable_visitor* v = cscript_new(ctxt, cscript_is_mutable_variable_visitor);
  v->visitor = cscript_visitor_new(ctxt, v);
  v->visitor->previsit_fixnum = previsit_fixnum;
  v->visitor->previsit_flonum = previsit_flonum;
  v->visitor->previsit_assignment = previsit_assignment;
  v->visitor->previsit_lvalueop = previsit_lvalueop;
  return v;
  }

static void cscript_is_mutable_variable_visitor_free(cscript_context* ctxt, cscript_is_mutable_variable_visitor* v)
  {
  if (v)
    {
    v->visitor->destroy(ctxt, v->visitor);
    cscript_delete(ctxt, v);
    }
  }

typedef struct cscript_replace_variable_visitor
  {
  cscript_visitor* visitor;
  cscript_string var_name;
  int replace_by_this_number_type;
  cscript_number replace_by_this_number;
  } cscript_replace_variable_visitor;

static int previsit_statement(cscript_context* ctxt, cscript_visitor* v, cscript_statement* s)
  {
  cscript_replace_variable_visitor* vis = (cscript_replace_variable_visitor*)(v->impl);
  if (s->type == cscript_statement_type_fixnum)
    {
    if (strcmp(s->statement.fixnum.name.string_ptr, vis->var_name.string_ptr) == 0)
      {
      cscript_statement_destroy(ctxt, s);
      s->type = cscript_statement_type_nop;
      s->statement.nop.filename = make_null_string();
      return 0;
      }
    }
  if (s->type == cscript_statement_type_flonum)
    {
    if (strcmp(s->statement.flonum.name.string_ptr, vis->var_name.string_ptr) == 0)
      {
      cscript_statement_destroy(ctxt, s);
      s->type = cscript_statement_type_nop;
      s->statement.nop.filename = make_null_string();
      return 0;
      }
    }
  return 1;
  }

static int previsit_factor(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_factor* f)
  {
  cscript_replace_variable_visitor* vis = (cscript_replace_variable_visitor*)(v->impl);
  if (f->type == cscript_factor_type_variable)
    {
    if (strcmp(f->factor.var.name.string_ptr, vis->var_name.string_ptr) == 0)
      {
      cscript_parsed_number nr;
      nr.filename = f->factor.var.filename;
      nr.line_nr = f->factor.var.line_nr;
      nr.column_nr = f->factor.var.column_nr;
      nr.number.fx = vis->replace_by_this_number.fx;
      nr.type = vis->replace_by_this_number_type;
      cscript_string_destroy(ctxt, &f->factor.var.name);
      cscript_vector_destroy(ctxt, &f->factor.var.dims);
      f->type = cscript_factor_type_number;
      f->factor.number = nr;
      return 0;
      }
    }
  return 1;
  }

static cscript_replace_variable_visitor* cscript_replace_variable_visitor_new(cscript_context* ctxt)
  {
  cscript_replace_variable_visitor* v = cscript_new(ctxt, cscript_replace_variable_visitor);
  v->visitor = cscript_visitor_new(ctxt, v);
  v->visitor->previsit_factor = previsit_factor;
  v->visitor->previsit_statement = previsit_statement;
  return v;
  }

static void cscript_replace_variable_visitor_free(cscript_context* ctxt, cscript_replace_variable_visitor* v)
  {
  if (v)
    {
    v->visitor->destroy(ctxt, v->visitor);
    cscript_delete(ctxt, v);
    }
  }

typedef struct cscript_constant_propagation_visitor
  {
  cscript_visitor* visitor;
  cscript_map* is_unmutable;
  cscript_program* program;
  } cscript_constant_propagation_visitor;

static void postvisit_fixnum(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_fixnum* fx)
  {
  cscript_constant_propagation_visitor* vis = (cscript_constant_propagation_visitor*)(v->impl);
  cscript_object key;
  key.type = cscript_object_type_string;
  key.value.s = fx->name;
  cscript_object* value = cscript_map_get(ctxt, vis->is_unmutable, &key);
  if (value != NULL && value->value.fx > 0) // unmutable fixnum
    {
    if (cscript_is_constant_expression(ctxt, &fx->expr))
      {
      cscript_vector result = cscript_get_constant_value_expression(ctxt, &fx->expr);
      if (result.vector_size == 1)
        {
        cscript_constant_value val = *cscript_vector_begin(&result, cscript_constant_value);
        cscript_replace_variable_visitor* repl = cscript_replace_variable_visitor_new(ctxt);
        cscript_string_copy(ctxt, &repl->var_name, &fx->name);
        repl->replace_by_this_number_type = cscript_number_type_fixnum;
        switch (val.type)
          {
          case cscript_number_type_fixnum:
            repl->replace_by_this_number.fx = val.number.fx;
            break;
          case cscript_number_type_flonum:
            repl->replace_by_this_number.fx = (cscript_fixnum)val.number.fl;
            break;
          }
        cscript_visit_program(ctxt, repl->visitor, vis->program);
        cscript_string_destroy(ctxt, &repl->var_name);
        cscript_replace_variable_visitor_free(ctxt, repl);        
        }
      cscript_vector_destroy(ctxt, &result);
      }
    }
  }

static void postvisit_flonum(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_flonum* fl)
  {
  cscript_constant_propagation_visitor* vis = (cscript_constant_propagation_visitor*)(v->impl);
  cscript_object key;
  key.type = cscript_object_type_string;
  key.value.s = fl->name;
  cscript_object* value = cscript_map_get(ctxt, vis->is_unmutable, &key);
  if (value != NULL && value->value.fx > 0) // unmutable fixnum
    {
    if (cscript_is_constant_expression(ctxt, &fl->expr))
      {
      cscript_vector result = cscript_get_constant_value_expression(ctxt, &fl->expr);
      if (result.vector_size == 1)
        {
        cscript_constant_value val = *cscript_vector_begin(&result, cscript_constant_value);
        cscript_replace_variable_visitor* repl = cscript_replace_variable_visitor_new(ctxt);
        cscript_string_copy(ctxt, &repl->var_name, &fl->name);
        repl->replace_by_this_number_type = cscript_number_type_flonum;
        switch (val.type)
          {
          case cscript_number_type_fixnum:
            repl->replace_by_this_number.fl = (cscript_flonum)val.number.fx;
            break;
          case cscript_number_type_flonum:
            repl->replace_by_this_number.fl = val.number.fl;
            break;
          }
        cscript_visit_program(ctxt, repl->visitor, vis->program);
        cscript_string_destroy(ctxt, &repl->var_name);
        cscript_replace_variable_visitor_free(ctxt, repl);
        }
      cscript_vector_destroy(ctxt, &result);
      }
    }
  }

static cscript_constant_propagation_visitor* cscript_constant_propagation_visitor_new(cscript_context* ctxt)
  {
  cscript_constant_propagation_visitor* v = cscript_new(ctxt, cscript_constant_propagation_visitor);
  v->visitor = cscript_visitor_new(ctxt, v);
  v->visitor->postvisit_fixnum = postvisit_fixnum;
  v->visitor->postvisit_flonum = postvisit_flonum;
  return v;
  }

static void cscript_constant_propagation_visitor_free(cscript_context* ctxt, cscript_constant_propagation_visitor* v)
  {
  if (v)
    {
    v->visitor->destroy(ctxt, v->visitor);
    cscript_delete(ctxt, v);
    }
  }

void cscript_constant_propagation(cscript_context* ctxt, cscript_program* program)
  {
  cscript_map* is_unmutable = cscript_map_new(ctxt, 0, 8);
  cscript_is_mutable_variable_visitor* v1 = cscript_is_mutable_variable_visitor_new(ctxt);
  v1->is_unmutable = is_unmutable;
  cscript_visit_program(ctxt, v1->visitor, program);
  cscript_is_mutable_variable_visitor_free(ctxt, v1);

  cscript_constant_propagation_visitor* v2 = cscript_constant_propagation_visitor_new(ctxt);
  v2->is_unmutable = is_unmutable;
  v2->program = program;
  cscript_visit_program(ctxt, v2->visitor, program);
  cscript_constant_propagation_visitor_free(ctxt, v2);

  cscript_map_free(ctxt, is_unmutable);
  }