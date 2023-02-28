#ifndef CSCRIPT_PRIMITIVES_H
#define CSCRIPT_PRIMITIVES_H

#include "cscript.h"
#include "map.h"

typedef enum
  {
  CSCRIPT_ADD
  } cscript_primitives;

void cscript_primitive_add(cscript_context* ctxt, int a, int b, int c);

void cscript_call_primitive(cscript_context* ctxt, cscript_fixnum prim_id, int a, int b, int c);

cscript_map* generate_primitives_map(cscript_context* ctxt);

#endif //CSCRIPT_PRIMITIVES_H