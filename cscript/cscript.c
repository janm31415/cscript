#include "cscript.h"
#include "func.h"
#include "compiler.h"
#include "token.h"
#include "parser.h"
#include "preprocess.h"
#include "error.h"
#include "context.h"
#include "environment.h"

#include <string.h>

cscript_function* cscript_compile(cscript_context* ctxt, const char* script)
  {
  cscript_syntax_errors_clear(ctxt);
  cscript_compile_errors_clear(ctxt);
  cscript_runtime_errors_clear(ctxt);
  assert(cscript_context_is_error_free(ctxt) != 0);
  cscript_vector tokens = cscript_script2tokens(ctxt, script);
  if (cscript_context_is_error_free(ctxt) == 0)
    {
    destroy_tokens_vector(ctxt, &tokens);
    return NULL;
    }
  cscript_program prog = make_program(ctxt, &tokens);
  if (cscript_context_is_error_free(ctxt) == 0)
    {
    destroy_tokens_vector(ctxt, &tokens);
    cscript_program_destroy(ctxt, &prog);
    return NULL;
    }
  cscript_preprocess(ctxt, &prog);
  if (cscript_context_is_error_free(ctxt) == 0)
    {
    destroy_tokens_vector(ctxt, &tokens);
    cscript_program_destroy(ctxt, &prog);
    return NULL;
    }
  cscript_function* fun = cscript_compile_program(ctxt, &prog);
  destroy_tokens_vector(ctxt, &tokens);
  cscript_program_destroy(ctxt, &prog);
  if (cscript_context_is_error_free(ctxt) == 0)
    {
    cscript_function_free(ctxt, fun);
    return NULL;
    }
  return fun;
  }

void cscript_get_error_message(cscript_context* ctxt, char* buffer, cscript_memsize buffer_size)
  {
  if (buffer_size < 2)
    {
    for (cscript_memsize i = 0; i < buffer_size; ++i)
      buffer[i] = 0;
    return;
    }
  cscript_string s;
  cscript_string_init(ctxt, &s, "");
  cscript_get_error_string(ctxt, &s);
  cscript_memsize len = s.string_length;
  if (len >= buffer_size)
    len = buffer_size-1;
  for (cscript_memsize i = 0; i < len; ++i)
    buffer[i] = s.string_ptr[i];
  buffer[len] = 0;
  cscript_string_destroy(ctxt, &s);
  }

void cscript_set_function_arguments(cscript_context* ctxt, cscript_fixnum* arguments, int number_of_arguments)
  {
  // fill stack with parameters
  for (int i = 0; i < number_of_arguments; ++i)
    {
    cscript_fixnum* fx = cscript_vector_begin(&ctxt->stack, cscript_fixnum) + i;
    *fx = arguments[i];
    }
  }

int cscript_set_global_flonum_value(cscript_context* ctxt, const char* global_name, cscript_flonum value)
  {
  if (global_name[0] != '$')
    {
    cscript_syntax_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, -1, -1, NULL, "globals should start with $");
    return 0;
    }
  cscript_environment_entry entry;
  cscript_string name;
  cscript_string_init(ctxt, &name, global_name);
  int found = cscript_environment_find_recursive(&entry, ctxt, &name);
  if (found)
    {
    memcpy(((cscript_fixnum*)ctxt->globals.vector_ptr) + entry.position, &value, sizeof(cscript_fixnum));
    }
  else
    {
    entry.type = CSCRIPT_ENV_TYPE_GLOBAL;
    entry.position = ctxt->globals.vector_size;
    entry.register_type = 1;
    cscript_string s;
    cscript_string_copy(ctxt, &s, &name);
    cscript_environment_add_to_base(ctxt, &s, entry);
    cscript_vector_push_back(ctxt, &ctxt->globals, value, cscript_flonum);
    }
  cscript_string_destroy(ctxt, &name);
  return 1;
  }

int cscript_set_global_fixnum_value(cscript_context* ctxt, const char* global_name, cscript_fixnum value)
  {
  if (global_name[0] != '$')
    {
    cscript_syntax_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, -1, -1, NULL, "globals should start with $");
    return 0;
    }
  cscript_environment_entry entry;
  cscript_string name;
  cscript_string_init(ctxt, &name, global_name);
  int found = cscript_environment_find_recursive(&entry, ctxt, &name);
  if (found)
    {
    memcpy(((cscript_fixnum*)ctxt->globals.vector_ptr) + entry.position, &value, sizeof(cscript_fixnum));
    }
  else
    {
    entry.type = CSCRIPT_ENV_TYPE_GLOBAL;
    entry.position = ctxt->globals.vector_size;
    entry.register_type = 0;
    cscript_string s;
    cscript_string_copy(ctxt, &s, &name);
    cscript_environment_add_to_base(ctxt, &s, entry);
    cscript_vector_push_back(ctxt, &ctxt->globals, value, cscript_fixnum);
    }
  cscript_string_destroy(ctxt, &name);
  return 1;
  }