#include "context.h"
#include "memory.h"
#include "error.h"

static cscript_context* context_new(cscript_context* ctxt)
  {
  cscript_byte* block = (cscript_byte*)cscript_malloc(ctxt, sizeof(cscript_context));
  if (block == NULL)
    return NULL;
  else
    return cast(cscript_context*, block);
  }

static void context_free(cscript_context* ctxt)
  {  
  cscript_syntax_errors_clear(ctxt);
  cscript_compile_errors_clear(ctxt);
  cscript_runtime_errors_clear(ctxt);
  cscript_vector_destroy(ctxt, &ctxt->syntax_error_reports);
  cscript_vector_destroy(ctxt, &ctxt->compile_error_reports);
  cscript_vector_destroy(ctxt, &ctxt->runtime_error_reports);
  cscript_string*  sit = cscript_vector_begin(&ctxt->filenames_list, cscript_string);
  cscript_string*  sit_end = cscript_vector_end(&ctxt->filenames_list, cscript_string);
  for (; sit != sit_end; ++sit)
    {
    cscript_string_destroy(ctxt, sit);
    }
  cscript_vector_destroy(ctxt, &ctxt->filenames_list);
  cscript_free(ctxt, ctxt, sizeof(cscript_context));
  }

static void context_init(cscript_context* ctxt, cscript_memsize heap_size)
  {
  cscript_assert(ctxt->global != NULL);  
  ctxt->error_jmp = NULL;
  ctxt->number_of_syntax_errors = 0;
  ctxt->number_of_compile_errors = 0;
  ctxt->number_of_syntax_errors = 0;
  cscript_vector_init(ctxt, &ctxt->syntax_error_reports, cscript_error_report);
  cscript_vector_init(ctxt, &ctxt->compile_error_reports, cscript_error_report);
  cscript_vector_init(ctxt, &ctxt->runtime_error_reports, cscript_error_report);
  cscript_vector_init(ctxt, &ctxt->filenames_list, cscript_string);
  }

cscript_context* cscript_open(cscript_memsize heap_size)
  {
  cscript_context* ctxt = context_new(NULL);
  if (ctxt)
    {
    cscript_global_context* g = cscript_new(NULL, cscript_global_context);
    ctxt->global = g;
    get_key(g->dummy_node)->type = cscript_object_type_undefined;
    get_value(g->dummy_node)->type = cscript_object_type_undefined;
    g->dummy_node->next = NULL;
    g->main_context = ctxt;  
    context_init(ctxt, heap_size);
    }
  return ctxt;
  }

void cscript_close(cscript_context* ctxt)
  {
  ctxt = ctxt->global->main_context; 
  cscript_free(ctxt, ctxt->global, sizeof(cscript_global_context));
  context_free(ctxt);
  }

cscript_context* cscript_context_init(cscript_context* ctxt, cscript_memsize heap_size)
  {
  cscript_assert(ctxt->global != NULL);
  cscript_context* ctxt_new = context_new(ctxt);
  if (ctxt_new)
    {
    ctxt_new->global = ctxt->global;
    context_init(ctxt_new, heap_size);
    }
  return ctxt_new;
  }

void cscript_context_destroy(cscript_context* ctxt)
  {
  cscript_assert(ctxt->global != NULL);
  cscript_assert(ctxt != ctxt->global->main_context);
  context_free(ctxt);
  }