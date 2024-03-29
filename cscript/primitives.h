#ifndef CSCRIPT_PRIMITIVES_H
#define CSCRIPT_PRIMITIVES_H

#include "cscript.h"
#include "map.h"

typedef enum
  {
  CSCRIPT_ADD_FIXNUM,
  CSCRIPT_ADD_FLONUM,
  CSCRIPT_SUB_FIXNUM,
  CSCRIPT_SUB_FLONUM,
  CSCRIPT_MUL_FIXNUM,
  CSCRIPT_MUL_FLONUM,
  CSCRIPT_DIV_FIXNUM,
  CSCRIPT_DIV_FLONUM,
  CSCRIPT_MOD_FIXNUM,
  CSCRIPT_MOD_FLONUM,
  CSCRIPT_LESS_FIXNUM,
  CSCRIPT_LESS_FLONUM,
  CSCRIPT_LEQ_FIXNUM,
  CSCRIPT_LEQ_FLONUM,
  CSCRIPT_GREATER_FIXNUM,
  CSCRIPT_GREATER_FLONUM,
  CSCRIPT_GEQ_FIXNUM,
  CSCRIPT_GEQ_FLONUM,
  CSCRIPT_EQUAL_FIXNUM,
  CSCRIPT_EQUAL_FLONUM,
  CSCRIPT_NOT_EQUAL_FIXNUM,
  CSCRIPT_NOT_EQUAL_FLONUM,
  CSCRIPT_SQRT_FLONUM,
  CSCRIPT_SIN_FLONUM,
  CSCRIPT_COS_FLONUM,
  CSCRIPT_EXP_FLONUM,
  CSCRIPT_LOG_FLONUM,
  CSCRIPT_LOG2_FLONUM,
  CSCRIPT_FABS_FLONUM,
  CSCRIPT_TAN_FLONUM,
  CSCRIPT_ATAN_FLONUM,
  CSCRIPT_ATAN2_FLONUM,
  CSCRIPT_POW_FLONUM,
  CSCRIPT_MIN_FLONUM,
  CSCRIPT_MAX_FLONUM,
  } cscript_primitives;

void cscript_primitive_add_fixnum(cscript_context* ctxt, int a);
void cscript_primitive_add_flonum(cscript_context* ctxt, int a);
void cscript_primitive_sub_fixnum(cscript_context* ctxt, int a);
void cscript_primitive_sub_flonum(cscript_context* ctxt, int a);
void cscript_primitive_mul_fixnum(cscript_context* ctxt, int a);
void cscript_primitive_mul_flonum(cscript_context* ctxt, int a);
void cscript_primitive_div_fixnum(cscript_context* ctxt, int a);
void cscript_primitive_div_flonum(cscript_context* ctxt, int a);
void cscript_primitive_mod_fixnum(cscript_context* ctxt, int a);
void cscript_primitive_mod_flonum(cscript_context* ctxt, int a);
void cscript_primitive_less_fixnum(cscript_context* ctxt, int a);
void cscript_primitive_less_flonum(cscript_context* ctxt, int a);
void cscript_primitive_leq_fixnum(cscript_context* ctxt, int a);
void cscript_primitive_leq_flonum(cscript_context* ctxt, int a);
void cscript_primitive_greater_fixnum(cscript_context* ctxt, int a);
void cscript_primitive_greater_flonum(cscript_context* ctxt, int a);
void cscript_primitive_geq_fixnum(cscript_context* ctxt, int a);
void cscript_primitive_geq_flonum(cscript_context* ctxt, int a);
void cscript_primitive_equal_fixnum(cscript_context* ctxt, int a);
void cscript_primitive_equal_flonum(cscript_context* ctxt, int a);
void cscript_primitive_not_equal_fixnum(cscript_context* ctxt, int a);
void cscript_primitive_not_equal_flonum(cscript_context* ctxt, int a);
void cscript_primitive_sqrt_flonum(cscript_context* ctxt, int a);
void cscript_primitive_sin_flonum(cscript_context* ctxt, int a);
void cscript_primitive_cos_flonum(cscript_context* ctxt, int a);
void cscript_primitive_exp_flonum(cscript_context* ctxt, int a);
void cscript_primitive_log_flonum(cscript_context* ctxt, int a);
void cscript_primitive_log2_flonum(cscript_context* ctxt, int a);
void cscript_primitive_fabs_flonum(cscript_context* ctxt, int a);
void cscript_primitive_tan_flonum(cscript_context* ctxt, int a);
void cscript_primitive_atan_flonum(cscript_context* ctxt, int a);
void cscript_primitive_atan2_flonum(cscript_context* ctxt, int a);
void cscript_primitive_pow_flonum(cscript_context* ctxt, int a);
void cscript_primitive_min_flonum(cscript_context* ctxt, int a);
void cscript_primitive_max_flonum(cscript_context* ctxt, int a);

void cscript_call_primitive(cscript_context* ctxt, cscript_fixnum prim_id, int a);

cscript_map* generate_primitives_map(cscript_context* ctxt);

#endif //CSCRIPT_PRIMITIVES_H