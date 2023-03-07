#include "alpha.h"
#include "context.h"
#include "visitor.h"
#include "string.h"
#include "map.h"
#include "syscalls.h"
#include <stdio.h>


cscript_string cscript_make_alpha_name(cscript_context* ctxt, cscript_string* original, cscript_memsize index)
  {
  char buffer[256];
  cscript_memsize_to_char(buffer, index);
  cscript_string new_name;
  cscript_string_init(ctxt, &new_name, "%");
  cscript_string_append(ctxt, &new_name, original);
  cscript_string_push_back(ctxt, &new_name, '_');
  cscript_string_append_cstr(ctxt, &new_name, buffer);
  return new_name;
  }

cscript_string cscript_get_original_name_from_alpha(cscript_context* ctxt, cscript_string* alpha_name)
  {
  char* it = cscript_string_begin(alpha_name);
  char* it_end = cscript_string_end(alpha_name);
  while (*it_end != '_')
    --it_end;
  ++it;
  cscript_string original;
  cscript_string_init_ranged(ctxt, &original, it, it_end);
  return original;
  }

typedef struct cscript_alpha_conversion_visitor
  {
  cscript_memsize index;
  cscript_vector variables; // linked chain of variable maps
  cscript_visitor* visitor;
  } cscript_alpha_conversion_visitor;

static void push_variables_child(cscript_context* ctxt, cscript_alpha_conversion_visitor* v)
  {
  cscript_map* child_vars = cscript_map_new(ctxt, 0, 3);
  cscript_vector_push_back(ctxt, &v->variables, child_vars, cscript_map*);
  }

static void pop_variables_child(cscript_context* ctxt, cscript_alpha_conversion_visitor* v)
  {
  cscript_map** child_map = cscript_vector_back(&v->variables, cscript_map*);
  cscript_map_keys_free(ctxt, *child_map);
  //cscript_map_values_free(ctxt, *child_map);
  cscript_map_free(ctxt, *child_map);
  cscript_vector_pop_back(&v->variables);
  }

static void add_variable(cscript_context* ctxt, cscript_alpha_conversion_visitor* v, cscript_string* var_name, cscript_string* alpha_name)
  {
  cscript_assert(v->variables.vector_size > 0);
  //cscript_string alpha_name = cscript_make_alpha_name(ctxt, var_name, v->index++);
  cscript_map** active_map = cscript_vector_back(&v->variables, cscript_map*);
  cscript_object key;
  key.type = cscript_object_type_string;
  key.value.s = *var_name;
  cscript_object* obj = cscript_map_insert(ctxt, *active_map, &key);
  obj->type = cscript_object_type_string;
  obj->value.s = *alpha_name;
  }

static int find_variable(cscript_string* alpha_name, cscript_context* ctxt, cscript_alpha_conversion_visitor* v, cscript_string* var_name)
  {
  cscript_object key;
  key.type = cscript_object_type_string;
  key.value.s = *var_name;
  cscript_map** map_it = cscript_vector_begin(&v->variables, cscript_map*);
  cscript_map** map_it_end = cscript_vector_end(&v->variables, cscript_map*);
  cscript_map** map_rit = map_it_end - 1;
  cscript_map** map_rit_end = map_it - 1;
  for (; map_rit != map_rit_end; --map_rit)
    {
    cscript_object* obj = cscript_map_get(ctxt, *map_rit, &key);
    if (obj != NULL)
      {
      *alpha_name = obj->value.s;
      return 1;
      }
    }
  return 0;
  }


static int previsit_scoped_statements(cscript_context* ctxt, cscript_visitor* v, cscript_scoped_statements* s)
  {
  UNUSED(s);
  cscript_alpha_conversion_visitor* vis = (cscript_alpha_conversion_visitor*)(v->impl);
  push_variables_child(ctxt, vis);
  return 1;
  }

static void postvisit_scoped_statements(cscript_context* ctxt, cscript_visitor* v, cscript_scoped_statements* s)
  {
  UNUSED(s);
  cscript_alpha_conversion_visitor* vis = (cscript_alpha_conversion_visitor*)(v->impl);
  pop_variables_child(ctxt, vis);
  }

static int previsit_assignment(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_assignment* s)
  {
  cscript_alpha_conversion_visitor* vis = (cscript_alpha_conversion_visitor*)(v->impl);
  cscript_string alpha_name;
  if (find_variable(&alpha_name, ctxt, vis, &s->name))
    {
    cscript_string_clear(&s->name);
    cscript_string_append(ctxt, &s->name, &alpha_name);
    }
  return 1;
  }

static int previsit_var(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_variable* s)
  {
  cscript_alpha_conversion_visitor* vis = (cscript_alpha_conversion_visitor*)(v->impl);
  cscript_string alpha_name;
  if (find_variable(&alpha_name, ctxt, vis, &s->name))
    {
    cscript_string_clear(&s->name);
    cscript_string_append(ctxt, &s->name, &alpha_name);
    }
  return 1;
  }

static int previsit_fixnum(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_fixnum* s)
  {
  cscript_alpha_conversion_visitor* vis = (cscript_alpha_conversion_visitor*)(v->impl);
  if (s->name.string_ptr[0] != '$')
    {
    cscript_string alpha_name = cscript_make_alpha_name(ctxt, &s->name, vis->index++);
    add_variable(ctxt, vis, &s->name, &alpha_name);
    s->name = alpha_name;
    }
  return 1;
  }

static int previsit_flonum(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_flonum* s)
  {
  cscript_alpha_conversion_visitor* vis = (cscript_alpha_conversion_visitor*)(v->impl);
  if (s->name.string_ptr[0] != '$')
    {
    cscript_string alpha_name = cscript_make_alpha_name(ctxt, &s->name, vis->index++);
    add_variable(ctxt, vis, &s->name, &alpha_name);
    s->name = alpha_name;
    }
  return 1;
  }

static void visit_parameter(cscript_context* ctxt, cscript_visitor* v, cscript_parameter* s)
  {
  cscript_alpha_conversion_visitor* vis = (cscript_alpha_conversion_visitor*)(v->impl);
  if (s->name.string_ptr[0] != '$')
    {
    cscript_string alpha_name = cscript_make_alpha_name(ctxt, &s->name, vis->index++);
    add_variable(ctxt, vis, &s->name, &alpha_name);
    s->name = alpha_name;
    }
  }

static cscript_alpha_conversion_visitor* cscript_alpha_conversion_visitor_new(cscript_context* ctxt)
  {
  cscript_alpha_conversion_visitor* v = cscript_new(ctxt, cscript_alpha_conversion_visitor);
  v->visitor = cscript_visitor_new(ctxt, v);
  v->visitor->previsit_var = previsit_var;
  v->visitor->previsit_assignment = previsit_assignment;
  v->visitor->previsit_fixnum = previsit_fixnum;
  v->visitor->previsit_flonum = previsit_flonum;
  v->visitor->previsit_scoped_statements = previsit_scoped_statements;
  v->visitor->postvisit_scoped_statements = postvisit_scoped_statements;
  v->visitor->visit_parameter = visit_parameter;
  return v;
  }

static void cscript_alpha_conversion_visitor_free(cscript_context* ctxt, cscript_alpha_conversion_visitor* v)
  {
  if (v)
    {
    cscript_vector_destroy(ctxt, &v->variables);
    v->visitor->destroy(ctxt, v->visitor);
    cscript_delete(ctxt, v);
    }
  }


void cscript_alpha_conversion(cscript_context* ctxt, cscript_program* program)
  {
  cscript_alpha_conversion_visitor* v = cscript_alpha_conversion_visitor_new(ctxt);
  v->index = 0;
  cscript_vector_init(ctxt, &v->variables, cscript_map*);
  push_variables_child(ctxt, v);
  cscript_visit_program(ctxt, v->visitor, program);
  pop_variables_child(ctxt, v);
  cscript_alpha_conversion_visitor_free(ctxt, v);
  }