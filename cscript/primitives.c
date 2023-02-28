#include "primitives.h"
#include "context.h"
#include "object.h"
#include "error.h"
#include "func.h"
#include "vm.h"
#include "syscalls.h"
#include "environment.h"
#include "dump.h"

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

////////////////////////////////////////////////////

void cscript_primitive_add(cscript_context* ctxt, int a, int b, int c)
  {
  }

void cscript_call_primitive(cscript_context* ctxt, cscript_fixnum function_id, int a, int b, int c)
  {
  switch (function_id)
    {
    case CSCRIPT_ADD:
      cscript_primitive_add(ctxt, a, b, c);
      break;  
    default:
      cscript_throw(ctxt, CSCRIPT_ERROR_NOT_IMPLEMENTED);
      break;
    }
  }

static void map_insert(cscript_context* ctxt, cscript_map* m, const char* str, int value)
  {
  cscript_object* obj = cscript_map_insert_string(ctxt, m, str);
  obj->type = cscript_object_type_fixnum;
  obj->value.fx = (cscript_fixnum)value;
  }

cscript_map* generate_primitives_map(cscript_context* ctxt)
  {
  cscript_map* m = cscript_map_new(ctxt, 0, 8);
  map_insert(ctxt, m, "+", CSCRIPT_ADD);  
  return m;
  }
