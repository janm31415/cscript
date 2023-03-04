#ifndef CSCRIPT_CONSTANT_H
#define CSCRIPT_CONSTANT_H

#include "cscript.h"
#include "parser.h"

int cscript_is_constant_factor(cscript_context* ctxt, cscript_parsed_factor* f);
int cscript_is_constant_term(cscript_context* ctxt, cscript_parsed_term* t);
int cscript_is_constant_relop(cscript_context* ctxt, cscript_parsed_relop* r);
int cscript_is_constant_expression(cscript_context* ctxt, cscript_parsed_expression* expr);
int cscript_is_constant_expression_list(cscript_context* ctxt, cscript_vector* exprs);

typedef struct cscript_constant_value
  {
  cscript_number number;
  int type;
  } cscript_constant_value;

cscript_vector cscript_get_constant_value_factor(cscript_context* ctxt, cscript_parsed_factor* f);
cscript_vector cscript_get_constant_value_term(cscript_context* ctxt, cscript_parsed_term* t);
cscript_vector cscript_get_constant_value_relop(cscript_context* ctxt, cscript_parsed_relop* r);
cscript_vector cscript_get_constant_value_expression(cscript_context* ctxt, cscript_parsed_expression* expr);

#endif //CSCRIPT_CONSTANT_H