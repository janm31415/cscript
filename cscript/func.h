#ifndef CSCRIPT_FUNC_H
#define CSCRIPT_FUNC_H

#include "cscript.h"
#include "vector.h"
#include "map.h"

typedef struct cscript_function
  {
  cscript_map* constants_map;
  cscript_vector constants;
  cscript_vector code;
  int number_of_constants;
  } cscript_function;


cscript_function* cscript_function_new(cscript_context* ctxt);
void cscript_function_free(cscript_context* ctxt, cscript_function* f);

#endif //CSCRIPT_FUNC_H