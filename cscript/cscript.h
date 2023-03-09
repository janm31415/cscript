#ifndef CSCRIPT_H
#define CSCRIPT_H

#include <stdint.h>

typedef struct cscript_context cscript_context;
typedef struct cscript_function cscript_function;

#ifndef CSCRIPT_FLONUM
typedef double cscript_flonum;
#else
typedef CSCRIPT_FLONUM cscript_flonum;
#endif

#ifndef CSCRIPT_FIXNUM
typedef int64_t cscript_fixnum;
#else
typedef CSCRIPT_FIXNUM cscript_fixnum;
#endif

#ifndef cscript_memsize
#define cscript_memsize uint32_t
#endif

typedef unsigned char cscript_byte;

#ifndef CSCRIPT_API
#define CSCRIPT_API extern
#endif

typedef enum cscript_foreign_return_type
  {
  cscript_foreign_flonum,
  cscript_foreign_fixnum,
  cscript_foreign_void
  } cscript_foreign_return_type;

CSCRIPT_API cscript_context* cscript_open(cscript_memsize stack_size);
CSCRIPT_API void cscript_close(cscript_context* ctxt);
CSCRIPT_API cscript_context* cscript_context_init(cscript_context* ctxt, cscript_memsize stack_size);
CSCRIPT_API void cscript_context_destroy(cscript_context* ctxt);

CSCRIPT_API void cscript_environment_clear(cscript_context* ctxt);

CSCRIPT_API void cscript_register_external_function(cscript_context* ctxt, const char* name, void* address, cscript_foreign_return_type ret_type);

#endif //CSCRIPT_H