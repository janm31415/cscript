#include "constant.h"

int cscript_is_constant_factor(const cscript_parsed_factor* f)
  {
  switch (f->type)
    {
    case cscript_factor_type_number:
      return 1;
    case cscript_factor_type_expression:
      return cscript_is_constant_expression(&f->factor.expr);
    }
  return 0;
  }

int cscript_is_constant_term(const cscript_parsed_term* t)
  {
  if (t->operands.vector_size != 1)
    return 0;
  return cscript_is_constant_factor(cscript_vector_begin(&t->operands, cscript_parsed_factor));
  }

int cscript_is_constant_relop(const cscript_parsed_relop* r)
  {
  if (r->operands.vector_size != 1)
    return 0;
  return cscript_is_constant_term(cscript_vector_begin(&r->operands, cscript_parsed_term));
  }

int cscript_is_constant_expression(const cscript_parsed_expression* expr)
  {
  if (expr->operands.vector_size != 1)
    return 0;
  return cscript_is_constant_relop(cscript_vector_begin(&expr->operands, cscript_parsed_relop));
  }

cscript_constant_value cscript_get_constant_value_factor(const cscript_parsed_factor* f)
  {
  cscript_constant_value value;
  value.number.fx = 0;
  value.type = cscript_number_type_fixnum;
  switch (f->type)
    {
    case cscript_factor_type_number:
      value.number = f->factor.number.number;
      value.type = f->factor.number.type;
      break;
    case cscript_factor_type_expression:
      return cscript_get_constant_value_expression(&f->factor.expr);
    default:
      cscript_assert(0);
    }
  if (f->sign == '-')
    {
    switch (value.type)
      {
      case cscript_number_type_fixnum:
        value.number.fx = -value.number.fx;
        break;
      case cscript_number_type_flonum:
        value.number.fl = -value.number.fl;
        break;
      }
    }
  return value;
  }

cscript_constant_value cscript_get_constant_value_term(const cscript_parsed_term* t)
  {
  return cscript_get_constant_value_factor(cscript_vector_begin(&t->operands, cscript_parsed_factor));
  }

cscript_constant_value cscript_get_constant_value_relop(const cscript_parsed_relop* r)
  {
  return cscript_get_constant_value_term(cscript_vector_begin(&r->operands, cscript_parsed_term));
  }

cscript_constant_value cscript_get_constant_value_expression(const cscript_parsed_expression* expr)
  {
  return cscript_get_constant_value_relop(cscript_vector_begin(&expr->operands, cscript_parsed_relop));
  }