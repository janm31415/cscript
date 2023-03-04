#include "constant.h"
#include "context.h"

#include <string.h>

static cscript_object* find_primitive(cscript_context* ctxt, cscript_string* s)
  {
  cscript_object key;
  key.type = cscript_object_type_string;
  key.value.s = *s;
  cscript_object* res = cscript_map_get(ctxt, ctxt->global->primitives_map, &key);
  return res;
  }

int cscript_is_constant_factor(cscript_context* ctxt, cscript_parsed_factor* f)
  {
  switch (f->type)
    {
    case cscript_factor_type_number:
      return 1;
    case cscript_factor_type_expression:
      return cscript_is_constant_expression(ctxt, &f->factor.expr);
      /*
    case cscript_factor_type_function:
    {
    cscript_object* fun = find_primitive(ctxt, &f->factor.fun.name);
    if (fun == NULL)
      return 0;
    return cscript_is_constant_expression_list(ctxt, &f->factor.fun.args);
    }
    */
    case cscript_factor_type_expression_list:
    {
    return cscript_is_constant_expression_list(ctxt, &f->factor.exprlist.expressions);
    }
    }
  return 0;
  }

int cscript_is_constant_term(cscript_context* ctxt, cscript_parsed_term* t)
  {
  if (t->operands.vector_size != 1)
    return 0;
  cscript_parsed_factor* f = cscript_vector_begin(&t->operands, cscript_parsed_factor);
  return cscript_is_constant_factor(ctxt, f);
  }

int cscript_is_constant_relop(cscript_context* ctxt, cscript_parsed_relop* r)
  {
  if (r->operands.vector_size != 1)
    return 0;
  cscript_parsed_term* t = cscript_vector_begin(&r->operands, cscript_parsed_term);
  return cscript_is_constant_term(ctxt, t);
  }

int cscript_is_constant_expression(cscript_context* ctxt, cscript_parsed_expression* expr)
  {
  if (expr->operands.vector_size != 1)
    return 0;
  cscript_parsed_relop* r = cscript_vector_begin(&expr->operands, cscript_parsed_relop);
  return cscript_is_constant_relop(ctxt, r);
  }

int cscript_is_constant_expression_list(cscript_context* ctxt, cscript_vector* exprs)
  {
  cscript_parsed_expression* it = cscript_vector_begin(exprs, cscript_parsed_expression);
  cscript_parsed_expression* it_end = cscript_vector_end(exprs, cscript_parsed_expression);
  for (; it != it_end; ++it)
    {
    int cst = cscript_is_constant_expression(ctxt, it);
    if (cst == 0)
      return 0;
    }
  return 1;
  }

cscript_vector cscript_get_constant_value_factor(cscript_context* ctxt, cscript_parsed_factor* f)
  {
  cscript_vector return_values;
  return_values.element_size = 0;
  return_values.vector_capacity = 0;
  return_values.vector_ptr = NULL;
  return_values.vector_size = 0;
  switch (f->type)
    {
    case cscript_factor_type_number:
    {
    cscript_vector_init(ctxt, &return_values, cscript_constant_value);
    cscript_constant_value value;
    value.number.fx = 0;
    value.type = cscript_number_type_fixnum;
    value.number = f->factor.number.number;
    value.type = f->factor.number.type;
    cscript_vector_push_back(ctxt, &return_values, value, cscript_constant_value);
    break;
    }
    case cscript_factor_type_expression:
    {
    return_values = cscript_get_constant_value_expression(ctxt, &f->factor.expr);
    break;
    }
    case cscript_factor_type_expression_list:
    {
    cscript_vector_init(ctxt, &return_values, cscript_constant_value);
    cscript_parsed_expression* it = cscript_vector_begin(&f->factor.exprlist.expressions, cscript_parsed_expression);
    cscript_parsed_expression* it_end = cscript_vector_end(&f->factor.exprlist.expressions, cscript_parsed_expression);
    for (; it != it_end; ++it)
      {
      cscript_vector val = cscript_get_constant_value_expression(ctxt, it);
      cscript_constant_value* insert_pos = cscript_vector_end(&return_values, cscript_constant_value);
      cscript_constant_value* range_it_begin = cscript_vector_begin(&val, cscript_constant_value);
      cscript_constant_value* range_it_end = cscript_vector_end(&val, cscript_constant_value);
      cscript_vector_insert(ctxt, &return_values, &insert_pos, &range_it_begin, &range_it_end, cscript_constant_value);
      cscript_vector_destroy(ctxt, &val);
      }
    break;
    }
    default:
      cscript_assert(0);
    }
  if (f->sign == '-')
    {
    cscript_constant_value* it = cscript_vector_begin(&return_values, cscript_constant_value);
    cscript_constant_value* it_end = cscript_vector_end(&return_values, cscript_constant_value);
    for (; it != it_end; ++it)
      {
      switch (it->type)
        {
        case cscript_number_type_fixnum:
          it->number.fx = -it->number.fx;
          break;
        case cscript_number_type_flonum:
          it->number.fl = -it->number.fl;
          break;
        }
      }
    }
  return return_values;
  }

cscript_vector cscript_get_constant_value_term(cscript_context* ctxt, cscript_parsed_term* t)
  {
  return cscript_get_constant_value_factor(ctxt, cscript_vector_begin(&t->operands, cscript_parsed_factor));
  }

cscript_vector cscript_get_constant_value_relop(cscript_context* ctxt, cscript_parsed_relop* r)
  {
  return cscript_get_constant_value_term(ctxt, cscript_vector_begin(&r->operands, cscript_parsed_term));
  }

cscript_vector cscript_get_constant_value_expression(cscript_context* ctxt, cscript_parsed_expression* expr)
  {
  return cscript_get_constant_value_relop(ctxt, cscript_vector_begin(&expr->operands, cscript_parsed_relop));
  }