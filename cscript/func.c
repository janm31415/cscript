#include "func.h"
#include "vector.h"
#include "object.h"

cscript_function* cscript_function_new(cscript_context* ctxt)
  {
  cscript_function* fun = cscript_new(ctxt, cscript_function);
  fun->constants_map = cscript_map_new(ctxt, 0, 8);
  cscript_vector_init(ctxt, &fun->constants, cscript_fixnum);
  cscript_vector_init(ctxt, &fun->code, cscript_instruction);
  fun->number_of_constants = 0;
  fun->result_position = 0;
  return fun;
  }

void cscript_function_free(cscript_context* ctxt, cscript_function* f)
  {
  cscript_map_free(ctxt, f->constants_map);
  cscript_vector_destroy(ctxt, &f->constants);
  cscript_vector_destroy(ctxt, &f->code);
  cscript_delete(ctxt, f);
  }