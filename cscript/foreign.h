#ifndef CSCRIPT_FOREIGN_H
#define CSCRIPT_FOREIGN_H

#include "cscript.h"
#include "string.h"
#include "vector.h"
#include "object.h"

typedef enum cscript_foreign_return_type
  {
  cscript_foreign_flonum,
  cscript_foreign_fixnum,
  cscript_foreign_void
  } cscript_foreign_return_type;

typedef struct cscript_external_function
  {
  cscript_string name;
  void* address;
  cscript_foreign_return_type return_type;
  } cscript_external_function;

cscript_external_function cscript_external_function_init(cscript_context* ctxt, const char* name, void* address, cscript_foreign_return_type ret_type);

void cscript_external_function_destroy(cscript_context* ctxt, cscript_external_function* ext);

void cscript_register_external_function(cscript_context* ctxt, cscript_external_function* ext);

cscript_object cscript_call_external(cscript_context* ctxt, cscript_external_function* ext, int argument_stack_offset, int nr_of_args);

#endif //CSCRIPT_FOREIGN_H