#ifndef CSCRIPT_LIMITS_H
#define CSCRIPT_LIMITS_H

#include "cscript.h"
#include <assert.h>
#include <limits.h>

#ifndef UNUSED
#define UNUSED(x) ((void)(x)) /* to avoid warnings */
#endif

#ifndef cast
#define cast(t, exp) ((t)(exp))
#endif

#ifndef cscript_assert
#define cscript_assert(c) assert(c)
#endif

#ifndef CSCRIPT_ALIGNMENT
typedef union { cscript_fixnum fx; cscript_flonum fl; void* ptr; } cscript_alignment;
#else
typedef CSCRIPT_ALIGNMENT cscript_alignment;
#endif

#ifndef cscript_memsize_sign_bit
#define cscript_memsize_sign_bit 0x80000000
#endif


#ifndef cscript_mem_bits
#define cscript_mem_bits 32
#endif

#ifndef cscript_mem_invalid_size
#define cscript_mem_invalid_size ((cscript_memsize)(-1))
#endif

#define cscript_swap(a, b, type) \
  { type tmp = (a); \
  (a) = (b); \
  (b) = tmp; }

typedef uint32_t cscript_instruction;

/* maximum stack for a scheme function */
#define cscript_maxstack	1024


#endif //CSCRIPT_LIMITS_H