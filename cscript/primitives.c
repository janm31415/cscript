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
  cscript_fixnum* rb = ra + 1;
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

void cscript_primitive_sqrt_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  *ra = sqrt(*ra);
  }

void cscript_primitive_sin_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  *ra = sin(*ra);
  }

void cscript_primitive_cos_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  *ra = cos(*ra);
  }

void cscript_primitive_exp_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  *ra = exp(*ra);
  }

void cscript_primitive_log_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  *ra = log(*ra);
  }

void cscript_primitive_log2_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  *ra = log2(*ra);
  }

void cscript_primitive_fabs_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  *ra = fabs(*ra);
  }

void cscript_primitive_tan_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  *ra = tan(*ra);
  }

void cscript_primitive_atan_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  *ra = atan(*ra);
  }

void cscript_primitive_atan2_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  cscript_flonum* rb = cscript_vector_at(&ctxt->stack, a + 1, cscript_flonum);
  *ra = atan2(*ra, *rb);
  }

void cscript_primitive_pow_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  cscript_flonum* rb = cscript_vector_at(&ctxt->stack, a + 1, cscript_flonum);
  *ra = pow(*ra, *rb);
  }

void cscript_primitive_min_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  cscript_flonum* rb = cscript_vector_at(&ctxt->stack, a + 1, cscript_flonum);
  *ra = *ra < *rb ? *ra : *rb;
  }

void cscript_primitive_max_flonum(cscript_context* ctxt, int a)
  {
  cscript_flonum* ra = cscript_vector_at(&ctxt->stack, a, cscript_flonum);
  cscript_flonum* rb = cscript_vector_at(&ctxt->stack, a + 1, cscript_flonum);
  *ra = *ra > *rb ? *ra : *rb;
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
    case CSCRIPT_SQRT_FLONUM:
      cscript_primitive_sqrt_flonum(ctxt, a);
      break;
    case CSCRIPT_SIN_FLONUM:
      cscript_primitive_sin_flonum(ctxt, a);
      break;
    case CSCRIPT_COS_FLONUM:
      cscript_primitive_cos_flonum(ctxt, a);
      break;
    case CSCRIPT_EXP_FLONUM:
      cscript_primitive_exp_flonum(ctxt, a);
      break;
    case CSCRIPT_LOG_FLONUM:
      cscript_primitive_log_flonum(ctxt, a);
      break;
    case CSCRIPT_LOG2_FLONUM:
      cscript_primitive_log2_flonum(ctxt, a);
      break;
    case CSCRIPT_FABS_FLONUM:
      cscript_primitive_fabs_flonum(ctxt, a);
      break;
    case CSCRIPT_TAN_FLONUM:
      cscript_primitive_tan_flonum(ctxt, a);
      break;
    case CSCRIPT_ATAN_FLONUM:
      cscript_primitive_atan_flonum(ctxt, a);
      break;
    case CSCRIPT_ATAN2_FLONUM:
      cscript_primitive_atan2_flonum(ctxt, a);
      break;
    case CSCRIPT_POW_FLONUM:
      cscript_primitive_pow_flonum(ctxt, a);
      break;
    case CSCRIPT_MIN_FLONUM:
      cscript_primitive_min_flonum(ctxt, a);
      break;
    case CSCRIPT_MAX_FLONUM:
      cscript_primitive_max_flonum(ctxt, a);
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
  map_insert(ctxt, m, "sqrt", CSCRIPT_SQRT_FLONUM);
  map_insert(ctxt, m, "sin", CSCRIPT_SIN_FLONUM);
  map_insert(ctxt, m, "cos", CSCRIPT_COS_FLONUM);
  map_insert(ctxt, m, "exp", CSCRIPT_EXP_FLONUM);
  map_insert(ctxt, m, "log", CSCRIPT_LOG_FLONUM);
  map_insert(ctxt, m, "log2", CSCRIPT_LOG2_FLONUM);
  map_insert(ctxt, m, "fabs", CSCRIPT_FABS_FLONUM);
  map_insert(ctxt, m, "tan", CSCRIPT_TAN_FLONUM);
  map_insert(ctxt, m, "atan", CSCRIPT_ATAN_FLONUM);
  map_insert(ctxt, m, "atan2", CSCRIPT_ATAN2_FLONUM);
  map_insert(ctxt, m, "pow", CSCRIPT_POW_FLONUM);
  map_insert(ctxt, m, "min", CSCRIPT_MIN_FLONUM);
  map_insert(ctxt, m, "max", CSCRIPT_MAX_FLONUM);
  return m;
  }
