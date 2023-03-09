#include "cscript.h"
#include "func.h"
#include "compiler.h"
#include "token.h"
#include "parser.h"
#include "preprocess.h"
#include "error.h"
#include "context.h"

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