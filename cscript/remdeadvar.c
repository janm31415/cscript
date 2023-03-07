#include "remdeadvar.h"
#include "context.h"
#include "visitor.h"
#include "constant.h"

#include <string.h>

typedef struct cscript_is_used_variable_visitor
  {
  cscript_visitor* visitor;
  cscript_map* is_unused;
  } cscript_is_used_variable_visitor;

static int previsit_fixnum(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_fixnum* fx)
  {
  cscript_is_used_variable_visitor* vis = (cscript_is_used_variable_visitor*)(v->impl);
  cscript_object key;
  key.type = cscript_object_type_string;
  cscript_string_copy(ctxt, &key.value.s, &fx->name);
  cscript_object* value = cscript_map_insert(ctxt, vis->is_unused, &key);
  value->type = cscript_object_type_fixnum;
  if (fx->name.string_ptr[0] == '$' || fx->dims.vector_size > 0)
    value->value.fx = 0;
  else
    value->value.fx = 1;
  return 1;
  }

static int previsit_flonum(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_flonum* fl)
  {
  cscript_is_used_variable_visitor* vis = (cscript_is_used_variable_visitor*)(v->impl);
  cscript_object key;
  key.type = cscript_object_type_string;
  cscript_string_copy(ctxt, &key.value.s, &fl->name);
  cscript_object* value = cscript_map_insert(ctxt, vis->is_unused, &key);
  value->type = cscript_object_type_fixnum;
  if (fl->name.string_ptr[0] == '$' || fl->dims.vector_size > 0)
    value->value.fx = 0;
  else
    value->value.fx = 1;
  return 1;
  }

static int previsit_lvalueop(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_lvalue_operator* l)
  {
  cscript_is_used_variable_visitor* vis = (cscript_is_used_variable_visitor*)(v->impl);
  cscript_object key;
  key.type = cscript_object_type_string;
  key.value.s = l->lvalue.name;
  cscript_object* value = cscript_map_get(ctxt, vis->is_unused, &key);
  if (value != NULL)
    {
    value->type = cscript_object_type_fixnum;
    value->value.fx = 0;
    }
  return 1;
  }

static int previsit_var(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_variable* var)
  {
  cscript_is_used_variable_visitor* vis = (cscript_is_used_variable_visitor*)(v->impl);
  cscript_object key;
  key.type = cscript_object_type_string;
  key.value.s = var->name;
  cscript_object* value = cscript_map_get(ctxt, vis->is_unused, &key);
  if (value != NULL)
    {
    value->type = cscript_object_type_fixnum;
    value->value.fx = 0;
    }
  return 1;
  }

static cscript_is_used_variable_visitor* cscript_is_used_variable_visitor_new(cscript_context* ctxt)
  {
  cscript_is_used_variable_visitor* v = cscript_new(ctxt, cscript_is_used_variable_visitor);
  v->visitor = cscript_visitor_new(ctxt, v);
  v->visitor->previsit_fixnum = previsit_fixnum;
  v->visitor->previsit_flonum = previsit_flonum;
  v->visitor->previsit_lvalueop = previsit_lvalueop;
  v->visitor->previsit_var = previsit_var;
  return v;
  }

static void cscript_is_used_variable_visitor_free(cscript_context* ctxt, cscript_is_used_variable_visitor* v)
  {
  if (v)
    {
    v->visitor->destroy(ctxt, v->visitor);
    cscript_delete(ctxt, v);
    }
  }

typedef struct cscript_remove_dead_variables_visitor
  {
  cscript_visitor* visitor;
  cscript_map* is_unused;
  cscript_program* program;
  } cscript_remove_dead_variables_visitor;

static int previsit_statement(cscript_context* ctxt, cscript_visitor* v, cscript_statement* s)
  {
  cscript_remove_dead_variables_visitor* vis = (cscript_remove_dead_variables_visitor*)(v->impl);
  switch (s->type)
    {
    case cscript_statement_type_fixnum:
    {
    cscript_object key;
    key.type = cscript_object_type_string;
    key.value.s = s->statement.fixnum.name;
    cscript_object* value = cscript_map_get(ctxt, vis->is_unused, &key);
    if (value != NULL && value->value.fx != 0)
      {
      cscript_statement_destroy(ctxt, s);
      s->type = cscript_statement_type_nop;
      s->statement.nop.filename = make_null_string();
      return 0;
      }
    break;
    }
    case cscript_statement_type_flonum:
    {
    cscript_object key;
    key.type = cscript_object_type_string;
    key.value.s = s->statement.flonum.name;
    cscript_object* value = cscript_map_get(ctxt, vis->is_unused, &key);
    if (value != NULL && value->value.fx != 0)
      {
      cscript_statement_destroy(ctxt, s);
      s->type = cscript_statement_type_nop;
      s->statement.nop.filename = make_null_string();
      return 0;
      }
    break;
    }
    case cscript_statement_type_assignment:
    {
    cscript_object key;
    key.type = cscript_object_type_string;
    key.value.s = s->statement.assignment.name;
    cscript_object* value = cscript_map_get(ctxt, vis->is_unused, &key);
    if (value != NULL && value->value.fx != 0)
      {
      cscript_statement_destroy(ctxt, s);
      s->type = cscript_statement_type_nop;
      s->statement.nop.filename = make_null_string();
      return 0;
      }
    break;
    }
    default:
      break;
    }
  return 1;
  }

static cscript_remove_dead_variables_visitor* cscript_remove_dead_variables_visitor_new(cscript_context* ctxt)
  {
  cscript_remove_dead_variables_visitor* v = cscript_new(ctxt, cscript_remove_dead_variables_visitor);
  v->visitor = cscript_visitor_new(ctxt, v);
  v->visitor->previsit_statement = previsit_statement;
  return v;
  }

static void cscript_remove_dead_variables_visitor_free(cscript_context* ctxt, cscript_remove_dead_variables_visitor* v)
  {
  if (v)
    {
    v->visitor->destroy(ctxt, v->visitor);
    cscript_delete(ctxt, v);
    }
  }

void cscript_remove_dead_variables(cscript_context* ctxt, cscript_program* program)
  {
  cscript_map* is_unused = cscript_map_new(ctxt, 0, 8);
  cscript_is_used_variable_visitor* v1 = cscript_is_used_variable_visitor_new(ctxt);
  v1->is_unused = is_unused;
  cscript_visit_program(ctxt, v1->visitor, program);
  cscript_is_used_variable_visitor_free(ctxt, v1);

  cscript_remove_dead_variables_visitor* v2 = cscript_remove_dead_variables_visitor_new(ctxt);
  v2->is_unused = is_unused;
  v2->program = program;
  cscript_visit_program(ctxt, v2->visitor, program);
  cscript_remove_dead_variables_visitor_free(ctxt, v2);

  cscript_map_keys_free(ctxt, is_unused);
  cscript_map_free(ctxt, is_unused);
  }