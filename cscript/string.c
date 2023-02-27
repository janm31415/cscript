#include "string.h"

#include <string.h>

void cscript_string_init(cscript_context* ctxt, cscript_string* str, const char* txt)
  {
  str->string_length = cast(cscript_memsize, strlen(txt));
  str->string_ptr = cscript_newvector(ctxt, str->string_length + 1, char);
  str->string_capacity = str->string_length + 1;
  memcpy(str->string_ptr, txt, str->string_capacity * sizeof(char));
  }

void cscript_string_init_with_size(cscript_context* ctxt, cscript_string* str, cscript_memsize size, char element)
  {
  str->string_length = size;
  str->string_ptr = cscript_newvector(ctxt, str->string_length + 1, char);
  str->string_capacity = str->string_length + 1;
  memset(str->string_ptr, element, size);
  str->string_ptr[size] = 0;
  }

void cscript_string_copy(cscript_context* ctxt, cscript_string* str, const cscript_string* str_to_copy)
  {
  str->string_length = str_to_copy->string_length;
  str->string_ptr = cscript_newvector(ctxt, str_to_copy->string_capacity, char);
  str->string_capacity = str_to_copy->string_capacity;
  memcpy(str->string_ptr, str_to_copy->string_ptr, str->string_capacity * sizeof(char));
  }

void cscript_string_init_ranged(cscript_context* ctxt, cscript_string* str, const char* from, const char* to)
  {
  str->string_length = cast(cscript_memsize, to-from);
  str->string_ptr = cscript_newvector(ctxt, str->string_length + 1, char);
  str->string_capacity = str->string_length + 1;
  memcpy(str->string_ptr, from, str->string_length * sizeof(char));
  str->string_ptr[str->string_length] = 0;
  }

void cscript_string_destroy(cscript_context* ctxt, cscript_string* str)
  {
  cscript_realloc(ctxt, str->string_ptr, str->string_capacity * sizeof(char), 0);
  }

char* cscript_string_at(cscript_string* str, cscript_memsize index)
  {
  return str->string_ptr + index;
  }

char* cscript_string_c_str(cscript_string* str)
  {
  return str->string_ptr;
  }

char* cscript_string_begin(cscript_string* str)
  {
  return str->string_ptr;
  }

char* cscript_string_end(cscript_string* str)
  {
  return str->string_ptr + str->string_length;
  }

char* cscript_string_front(cscript_string* str)
  {
  return str->string_ptr;
  }

char* cscript_string_back(cscript_string* str)
  {
  return str->string_ptr + (str->string_length - 1);
  }

void cscript_string_push_front(cscript_context* ctxt, cscript_string* str, char ch)
  {
  cscript_growvector(ctxt, str->string_ptr, str->string_length + 2, str->string_capacity, char);
  memmove(str->string_ptr+1, str->string_ptr, str->string_length);
  *(str->string_ptr) = ch;
  ++(str->string_length);
  *(str->string_ptr + str->string_length) = 0;
  }

void cscript_string_push_back(cscript_context* ctxt, cscript_string* str, char ch)
  {
  cscript_growvector(ctxt, str->string_ptr, str->string_length + 2, str->string_capacity, char);
  *(str->string_ptr + str->string_length) = ch;
  ++(str->string_length);
  *(str->string_ptr + str->string_length) = 0;
  }

void cscript_string_pop_back(cscript_string* str)
  {
  if (str->string_length > 0)
    {
    --str->string_length;
    *(str->string_ptr + str->string_length) = 0;
    }
  }

void cscript_string_append(cscript_context* ctxt, cscript_string* str, cscript_string* append)
  {
  //cscript_growvector(ctxt, str->string_ptr, str->string_length + append->string_length + 1, str->string_capacity, char);
  cscript_reallocvector(ctxt, str->string_ptr, str->string_capacity, str->string_length + append->string_length + 1, char);
  memcpy(str->string_ptr + str->string_length, append->string_ptr, (append->string_length+1) * sizeof(char));
  str->string_capacity = str->string_length + append->string_length + 1;
  str->string_length += append->string_length;
  }

void cscript_string_append_cstr(cscript_context* ctxt, cscript_string* str, const char* append)
  {
  cscript_memsize append_length = cast(cscript_memsize, strlen(append));
  cscript_reallocvector(ctxt, str->string_ptr, str->string_capacity, str->string_length + append_length + 1, char);
  memcpy(str->string_ptr + str->string_length, append, (append_length + 1) * sizeof(char));
  str->string_capacity = str->string_length + append_length + 1;
  str->string_length += append_length;  
  }

void cscript_string_clear(cscript_string* str)
  {
  if (str->string_capacity > 0)
    {
    str->string_ptr[0] = 0;
    str->string_length = 0;
    }
  }

cscript_string make_null_string()
  {
  cscript_string s;
  s.string_capacity = 0;
  s.string_length = 0;
  s.string_ptr = NULL;
  return s;
  }

int cscript_string_compare_less(cscript_string* left, cscript_string* right)
  {
  const cscript_memsize size = left->string_length < right->string_length ? left->string_length : right->string_length;
  for (cscript_memsize i = 0; i < size; ++i)
    {
    if (left->string_ptr[i] != right->string_ptr[i])
      return left->string_ptr[i] < right->string_ptr[i] ? 1 : 0;
    }
  return left->string_length < right->string_length ? 1 : 0;
  }