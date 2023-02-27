#ifndef CSCRIPT_OBJECT_H
#define CSCRIPT_OBJECT_H

#include "cscript.h"
#include "limits.h"
#include "string.h"
#include "vector.h"

#define cscript_object_type_undefined 0
#define cscript_object_type_fixnum 1
#define cscript_object_type_flonum 2
#define cscript_object_type_string 3

typedef union
  {
  cscript_vector v;
  cscript_string s;
  void* ptr;
  cscript_fixnum fx;
  cscript_flonum fl;    
  cscript_byte ch;
  } cscript_value;

typedef struct cscript_object
  {  
  cscript_value value;
  int type;
  } cscript_object;

#define cscript_object_get_type(obj) \
  ((obj)->type)

#define cscript_set_object(obj1, obj2) \
  { \
  *(obj1) = *(obj2); \
  }

int cscript_objects_equal(cscript_context* ctxt, const cscript_object* obj1, const cscript_object* obj2);

int cscript_log2(uint32_t x);

cscript_object make_cscript_object_fixnum(cscript_fixnum fx);
cscript_object make_cscript_object_flonum(cscript_flonum fl);
cscript_object make_cscript_object_string(cscript_context* ctxt, const char* s);

void cscript_object_destroy(cscript_context* ctxt, cscript_object* obj);

void cscript_object_append_to_string(cscript_context* ctxt, cscript_object* obj, cscript_string* s, int display);
cscript_string cscript_object_to_string(cscript_context* ctxt, cscript_object* obj, int display);

#endif //CSCRIPT_OBJECT_H