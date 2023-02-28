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

void cscript_primitive_add_fixnum(cscript_context* ctxt, int a)
  {
  cscript_fixnum* ra = cscript_vector_at(&ctxt->stack, a, cscript_fixnum);
  cscript_fixnum* rb = ra+1;
  *ra += *rb;
  }

void cscript_primitive_add_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  cscript_flonum* rb = ra + 1;
  *ra += *rb;
  }

void cscript_primitive_sub_fixnum(cscript_context* ctxt, int a)
  {
  cscript_fixnum* ra = cscript_vector_at(&ctxt->stack, a, cscript_fixnum);
  cscript_fixnum* rb = ra + 1;
  *ra -= *rb;
  }

void cscript_primitive_sub_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  cscript_flonum* rb = ra + 1;
  *ra -= *rb;
  }

void cscript_primitive_mul_fixnum(cscript_context* ctxt, int a)
  {
  cscript_fixnum* ra = cscript_vector_at(&ctxt->stack, a, cscript_fixnum);
  cscript_fixnum* rb = ra + 1;
  *ra *= *rb;
  }

void cscript_primitive_mul_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  cscript_flonum* rb = ra + 1;
  *ra *= *rb;
  }

void cscript_primitive_div_fixnum(cscript_context* ctxt, int a)
  {
  cscript_fixnum* ra = cscript_vector_at(&ctxt->stack, a, cscript_fixnum);
  cscript_fixnum* rb = ra + 1;
  *ra /= *rb;
  }

void cscript_primitive_div_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  cscript_flonum* rb = ra + 1;
  *ra /= *rb;
  }

void cscript_primitive_mod_fixnum(cscript_context* ctxt, int a)
  {
  cscript_fixnum* ra = cscript_vector_at(&ctxt->stack, a, cscript_fixnum);
  cscript_fixnum* rb = ra + 1;
  *ra = *ra % *rb;
  }

void cscript_primitive_mod_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  cscript_flonum* rb = ra + 1;
  *ra = fmod(*ra, *rb);
  }

void cscript_primitive_less_fixnum(cscript_context* ctxt, int a)
  {
  cscript_fixnum* ra = cscript_vector_at(&ctxt->stack, a, cscript_fixnum);
  cscript_fixnum* rb = ra + 1;
  *ra = *ra < *rb ? 1 : 0;
  }

void cscript_primitive_less_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  cscript_flonum* rb = ra + 1;
  if (*ra < *rb)
    {
    *cast(cscript_fixnum*, ra) = 1;
    }
  else
    {
    *cast(cscript_fixnum*, ra) = 0;
    }
  }

void cscript_primitive_leq_fixnum(cscript_context* ctxt, int a)
  {
  cscript_fixnum* ra = cscript_vector_at(&ctxt->stack, a, cscript_fixnum);
  cscript_fixnum* rb = ra + 1;
  *ra = *ra <= *rb ? 1 : 0;
  }

void cscript_primitive_leq_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  cscript_flonum* rb = ra + 1;
  if (*ra <= *rb)
    {
    *cast(cscript_fixnum*, ra) = 1;
    }
  else
    {
    *cast(cscript_fixnum*, ra) = 0;
    }
  }

void cscript_primitive_greater_fixnum(cscript_context* ctxt, int a)
  {
  cscript_fixnum* ra = cscript_vector_at(&ctxt->stack, a, cscript_fixnum);
  cscript_fixnum* rb = ra + 1;
  *ra = *ra > *rb ? 1 : 0;
  }

void cscript_primitive_greater_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  cscript_flonum* rb = ra + 1;
  if (*ra > *rb)
    {
    *cast(cscript_fixnum*, ra) = 1;
    }
  else
    {
    *cast(cscript_fixnum*, ra) = 0;
    }
  }

void cscript_primitive_geq_fixnum(cscript_context* ctxt, int a)
  {
  cscript_fixnum* ra = cscript_vector_at(&ctxt->stack, a, cscript_fixnum);
  cscript_fixnum* rb = ra + 1;
  *ra = *ra >= *rb ? 1 : 0;
  }

void cscript_primitive_geq_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  cscript_flonum* rb = ra + 1;
  if (*ra >= *rb)
    {
    *cast(cscript_fixnum*, ra) = 1;
    }
  else
    {
    *cast(cscript_fixnum*, ra) = 0;
    }
  }

void cscript_primitive_equal_fixnum(cscript_context* ctxt, int a)
  {
  cscript_fixnum* ra = cscript_vector_at(&ctxt->stack, a, cscript_fixnum);
  cscript_fixnum* rb = ra + 1;
  *ra = *ra == *rb ? 1 : 0;
  }

void cscript_primitive_equal_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  cscript_flonum* rb = ra + 1;
  if (*ra == *rb)
    {
    *cast(cscript_fixnum*, ra) = 1;
    }
  else
    {
    *cast(cscript_fixnum*, ra) = 0;
    }
  }

void cscript_primitive_not_equal_fixnum(cscript_context* ctxt, int a)
  {
  cscript_fixnum* ra = cscript_vector_at(&ctxt->stack, a, cscript_fixnum);
  cscript_fixnum* rb = ra + 1;
  *ra = *ra != *rb ? 1 : 0;
  }

void cscript_primitive_not_equal_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  cscript_flonum* rb = ra + 1;
  if (*ra != *rb)
    {
    *cast(cscript_fixnum*, ra) = 1;
    }
  else
    {
    *cast(cscript_fixnum*, ra) = 0;
    }
  }

void cscript_call_primitive(cscript_context* ctxt, cscript_fixnum function_id, int a)
  {
  switch (function_id)
    {
    case CSCRIPT_ADD_FIXNUM:
      cscript_primitive_add_fixnum(ctxt, a);
      break;  
    case CSCRIPT_ADD_FLONUM:
      cscript_primitive_add_flonum(ctxt, a);
      break;
    case CSCRIPT_SUB_FIXNUM:
      cscript_primitive_sub_fixnum(ctxt, a);
      break;
    case CSCRIPT_SUB_FLONUM:
      cscript_primitive_sub_flonum(ctxt, a);
      break;
    case CSCRIPT_MUL_FIXNUM:
      cscript_primitive_mul_fixnum(ctxt, a);
      break;
    case CSCRIPT_MUL_FLONUM:
      cscript_primitive_mul_flonum(ctxt, a);
      break;
    case CSCRIPT_DIV_FIXNUM:
      cscript_primitive_div_fixnum(ctxt, a);
      break;
    case CSCRIPT_DIV_FLONUM:
      cscript_primitive_div_flonum(ctxt, a);
      break;
    case CSCRIPT_MOD_FIXNUM:
      cscript_primitive_mod_fixnum(ctxt, a);
      break;
    case CSCRIPT_MOD_FLONUM:
      cscript_primitive_mod_flonum(ctxt, a);
      break;
    case CSCRIPT_LESS_FIXNUM:
      cscript_primitive_less_fixnum(ctxt, a);
      break;
    case CSCRIPT_LESS_FLONUM:
      cscript_primitive_less_flonum(ctxt, a);
      break;
    case CSCRIPT_LEQ_FIXNUM:
      cscript_primitive_leq_fixnum(ctxt, a);
      break;
    case CSCRIPT_LEQ_FLONUM:
      cscript_primitive_leq_flonum(ctxt, a);
      break;
    case CSCRIPT_GREATER_FIXNUM:
      cscript_primitive_greater_fixnum(ctxt, a);
      break;
    case CSCRIPT_GREATER_FLONUM:
      cscript_primitive_greater_flonum(ctxt, a);
      break;
    case CSCRIPT_GEQ_FIXNUM:
      cscript_primitive_geq_fixnum(ctxt, a);
      break;
    case CSCRIPT_GEQ_FLONUM:
      cscript_primitive_geq_flonum(ctxt, a);
      break;
    case CSCRIPT_EQUAL_FIXNUM:
      cscript_primitive_equal_fixnum(ctxt, a);
      break;
    case CSCRIPT_EQUAL_FLONUM:
      cscript_primitive_equal_flonum(ctxt, a);
      break;
    case CSCRIPT_NOT_EQUAL_FIXNUM:
      cscript_primitive_not_equal_fixnum(ctxt, a);
      break;
    case CSCRIPT_NOT_EQUAL_FLONUM:
      cscript_primitive_not_equal_flonum(ctxt, a);
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
  //map_insert(ctxt, m, "+", CSCRIPT_ADD);  
  return m;
  }
