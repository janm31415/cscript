#ifndef CSCRIPT_MEMORY_H
#define CSCRIPT_MEMORY_H

#include "cscript.h"
#include "limits.h"

#define CSCRIPT_MINSIZEVECTOR 2

CSCRIPT_API void* cscript_realloc(cscript_context* ctxt, void* chunk, cscript_memsize old_size, cscript_memsize new_size);

#define cscript_free(ctxt, chunk, chunksize) cscript_realloc(ctxt, (chunk), (chunksize), 0)
#define cscript_delete(ctxt, chunk) cscript_realloc(ctxt, (chunk), sizeof(*(chunk)), 0)
#define cscript_freevector(ctxt, chunk, vector_size, element_type)	cscript_realloc(ctxt, (chunk), cast(cscript_memsize, vector_size)*cast(cscript_memsize, sizeof(element_type)), 0)

#define cscript_malloc(ctxt, size) cscript_realloc(ctxt, NULL, 0, (size))
#define cscript_new(ctxt, element_type) cast(element_type*, cscript_malloc(ctxt, sizeof(element_type)))
#define cscript_newvector(ctxt, vector_size, element_type) cast(element_type*, cscript_malloc(ctxt, cast(cscript_memsize, vector_size)*cast(cscript_memsize, sizeof(element_type))))

#define cscript_reallocvector(ctxt, vec, old_vector_size, new_vector_size, element_type) \
   ((vec)=cast(element_type*, cscript_realloc(ctxt, vec, cast(cscript_memsize, old_vector_size)*cast(cscript_memsize, sizeof(element_type)), \
          cast(cscript_memsize, new_vector_size)*cast(cscript_memsize, sizeof(element_type)))))

void* cscript_growvector_aux(cscript_context* ctxt, void* chunk, cscript_memsize* size, cscript_memsize element_size);

#define cscript_growvector(ctxt, vec, number_of_elements, vector_size, element_type) \
          if ((number_of_elements) > (vector_size)) \
            ((vec)=cast(element_type*, cscript_growvector_aux(ctxt, vec, &(vector_size), sizeof(element_type))))

#endif //CSCRIPT_MEMORY_H