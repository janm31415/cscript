#include "memory.h"
#include "error.h"

#include <stdlib.h>


/*
** definition for realloc function. It must assure that cscript_realloc(NULL,
** 0, x) allocates a new block (ANSI C assures that). ('old_size' is the old
** block size; some allocators may use that.)
*/
#ifndef CSCRIPT_REALLOC
#ifdef _WIN32
#define CSCRIPT_REALLOC(ptr, old_size, size) _aligned_realloc(ptr, size, 32)
#else
#define CSCRIPT_REALLOC(ptr, old_size, size) realloc(ptr, size)
#endif
#endif

/*
** definition for free function. ('old_size' is the old block size; some
** allocators may use that.)
*/
#ifndef CSCRIPT_FREE
#ifdef _WIN32
#define CSCRIPT_FREE(ptr,old_size) _aligned_free(ptr)
#else
#define CSCRIPT_FREE(ptr,old_size) free(ptr)
#endif
#endif

void* cscript_realloc(cscript_context* ctxt, void* chunk, cscript_memsize old_size, cscript_memsize new_size)
  {
  UNUSED(old_size);
  cscript_assert((old_size == 0) == (chunk == NULL));
  if (new_size == 0)
    {
    if (chunk != NULL)
      {
      CSCRIPT_FREE(chunk, old_size);
      chunk = NULL;
      }
    else return NULL;
    }
  else
    {
    chunk = CSCRIPT_REALLOC(chunk, old_size, new_size);
    if (chunk == NULL)
      {
      if (ctxt)
        cscript_throw(ctxt, CSCRIPT_ERROR_MEMORY);
      else
        return NULL;
      }
    }

  return chunk;
  }

void* cscript_growvector_aux(cscript_context* ctxt, void* chunk, cscript_memsize* size, cscript_memsize element_size)
  {
  void* newblock;
  cscript_memsize newsize = (*size) * 2;
  if (newsize < CSCRIPT_MINSIZEVECTOR)
    newsize = CSCRIPT_MINSIZEVECTOR;  /* minimum size */
  newblock = cscript_realloc(ctxt, chunk,
    *size * element_size,
    newsize * element_size);
  *size = newsize;  /* update only when everything else is OK */
  return newblock;
  }
