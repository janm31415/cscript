#ifndef CSCRIPT_CONSTANT_H
#define CSCRIPT_CONSTANT_H

#include "cscript.h"
#include "parser.h"

int cscript_is_constant_factor(const cscript_parsed_factor* f);
int cscript_is_constant_term(const cscript_parsed_term* t);
int cscript_is_constant_relop(const cscript_parsed_relop* r);
int cscript_is_constant_expression(const cscript_parsed_expression* expr);

typedef struct cscript_constant_value
  {
  cscript_number number;
  int type;
  } cscript_constant_value;

cscript_constant_value cscript_get_constant_value_factor(const cscript_parsed_factor* f);
cscript_constant_value cscript_get_constant_value_term(const cscript_parsed_term* t);
cscript_constant_value cscript_get_constant_value_relop(const cscript_parsed_relop* r);
cscript_constant_value cscript_get_constant_value_expression(const cscript_parsed_expression* expr);

#endif //CSCRIPT_CONSTANT_H