#ifndef CSCRIPT_STRING_H
#define CSCRIPT_STRING_H

#include "cscript.h"
#include "memory.h"

typedef struct cscript_string
  {
  char* string_ptr;
  cscript_memsize string_capacity; // the memory capacity of the string
  cscript_memsize string_length; // the length of the string  
  } cscript_string;

CSCRIPT_API void cscript_string_init(cscript_context* ctxt, cscript_string* str, const char* txt);

CSCRIPT_API void cscript_string_init_with_size(cscript_context* ctxt, cscript_string* str, cscript_memsize size, char element);

CSCRIPT_API void cscript_string_copy(cscript_context* ctxt, cscript_string* str, const cscript_string* str_to_copy);

CSCRIPT_API void cscript_string_init_ranged(cscript_context* ctxt, cscript_string* str, const char* from, const char* to);

CSCRIPT_API void cscript_string_destroy(cscript_context* ctxt, cscript_string* str);

CSCRIPT_API char* cscript_string_at(cscript_string* str, cscript_memsize index);

CSCRIPT_API char* cscript_string_c_str(cscript_string* str);

CSCRIPT_API char* cscript_string_begin(cscript_string* str);

CSCRIPT_API char* cscript_string_end(cscript_string* str);

CSCRIPT_API char* cscript_string_front(cscript_string* str);

CSCRIPT_API char* cscript_string_back(cscript_string* str);

CSCRIPT_API void cscript_string_push_front(cscript_context* ctxt, cscript_string* str, char ch);

CSCRIPT_API void cscript_string_push_back(cscript_context* ctxt, cscript_string* str, char ch);

CSCRIPT_API void cscript_string_pop_back(cscript_string* str);

CSCRIPT_API void cscript_string_append(cscript_context* ctxt, cscript_string* str, cscript_string* append);

CSCRIPT_API void cscript_string_append_cstr(cscript_context* ctxt, cscript_string* str, const char* append);

CSCRIPT_API void cscript_string_clear(cscript_string* str);

CSCRIPT_API cscript_string make_null_string();

CSCRIPT_API int cscript_string_compare_less(cscript_string* left, cscript_string* right);

#endif // CSCRIPT_STRING_H