#ifndef CSCRIPT_H
#define CSCRIPT_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>


typedef struct cscript_context cscript_context;

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

CSCRIPT_API cscript_context* cscript_open(cscript_memsize heap_size);
CSCRIPT_API void cscript_close(cscript_context* ctxt);
CSCRIPT_API cscript_context* cscript_context_init(cscript_context* ctxt, cscript_memsize heap_size);
CSCRIPT_API void cscript_context_destroy(cscript_context* ctxt);

#endif //CSCRIPT_H