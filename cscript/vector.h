#ifndef CSCRIPT_VECTOR_H
#define CSCRIPT_VECTOR_H

#include "cscript.h"
#include "memory.h"

typedef struct cscript_vector
  {
  void* vector_ptr;
  cscript_memsize vector_capacity; // the memory capacity of the vector
  cscript_memsize vector_size; // the number of elements in the vector
  cscript_memsize element_size; // the size of the elements in the vector
  } cscript_vector;

#define cscript_vector_init_with_size(ctxt, vec, size, element_type) \
  (vec)->vector_ptr = cscript_newvector(ctxt, size, element_type); \
  (vec)->vector_capacity = cast(cscript_memsize, size); \
  (vec)->vector_size = cast(cscript_memsize, size); \
  (vec)->element_size = sizeof(element_type)

#define cscript_vector_init(ctxt, vec, element_type) \
  (vec)->vector_ptr = cscript_newvector(ctxt, CSCRIPT_MINSIZEVECTOR, element_type); \
  (vec)->vector_capacity = cast(cscript_memsize, CSCRIPT_MINSIZEVECTOR); \
  (vec)->vector_size = 0; \
  (vec)->element_size = sizeof(element_type)

#define cscript_vector_init_reserve(ctxt, vec, size, element_type) \
  (vec)->vector_ptr = cscript_newvector(ctxt, size, element_type); \
  (vec)->vector_capacity = cast(cscript_memsize, size); \
  (vec)->vector_size = 0; \
  (vec)->element_size = sizeof(element_type)

#define cscript_vector_destroy(ctxt, vec) \
  cscript_realloc(ctxt, (vec)->vector_ptr, (vec)->vector_capacity*(vec)->element_size, 0)

#define cscript_vector_at(vec, index, element_type) \
  (cast(element_type*, (vec)->vector_ptr) + (index))

#define cscript_vector_pop_back(vec) \
  if ((vec)->vector_size > 0) --((vec)->vector_size)

#define cscript_vector_pop_front(vec) \
  if ((vec)->vector_size > 0) { \
    --((vec)->vector_size); \
    memmove(cast(char*, (vec)->vector_ptr), cast(char*, (vec)->vector_ptr)+(vec)->element_size, (vec)->vector_size*(vec)->element_size); \
  }

#define cscript_vector_push_back(ctxt, vec, element_value, element_type) \
  cscript_growvector(ctxt, (vec)->vector_ptr, (vec)->vector_size+1, (vec)->vector_capacity, element_type); \
  *cscript_vector_at(vec, (vec)->vector_size, element_type) = (element_value); \
  ++((vec)->vector_size)

#define cscript_vector_push_front(ctxt, vec, element_value, element_type) \
  cscript_growvector(ctxt, (vec)->vector_ptr, (vec)->vector_size+1, (vec)->vector_capacity, element_type); \
  memmove(cast(element_type*, (vec)->vector_ptr)+1, cast(element_type*, (vec)->vector_ptr), (vec)->vector_size*sizeof(element_type)); \
  *cscript_vector_at(vec, 0, element_type) = (element_value); \
  ++((vec)->vector_size)

#define cscript_vector_begin(vec, element_type) \
  cast(element_type*, (vec)->vector_ptr)

#define cscript_vector_end(vec, element_type) \
  (cast(element_type*, (vec)->vector_ptr) + (vec)->vector_size)

#define cscript_vector_back(vec, element_type) \
  (cast(element_type*, (vec)->vector_ptr) + ((vec)->vector_size - 1))

#define cscript_vector_erase(vec, iterator, element_type) \
  { element_type* it_end_obfuscate_asdfasdfds = (cast(element_type*, (vec)->vector_ptr) + (vec)->vector_size); \
  memmove(*(iterator), cast(element_type*, *(iterator))+1, (size_t)(it_end_obfuscate_asdfasdfds-cast(element_type*, *(iterator))-1)*sizeof(element_type)); }\
  --((vec)->vector_size)

#define cscript_vector_erase_range(vec, first, last, element_type) \
  { element_type* it_end_obfuscate_asdfasdfds = (cast(element_type*, (vec)->vector_ptr) + (vec)->vector_size); \
  memmove(*(first), *(last), (size_t)(it_end_obfuscate_asdfasdfds-*(last))*sizeof(element_type)); }\
  (vec)->vector_size -= cast(cscript_memsize, *(last)-*(first));

#define cscript_vector_insert(ctxt, vec, iterator, range_it_begin, range_it_end, element_type) \
  { cscript_memsize range_size = cast(cscript_memsize, cast(element_type*, *(range_it_end))-cast(element_type*, *(range_it_begin))); \
    cscript_memsize mem_needed = (vec)->vector_size + range_size; \
    if ((vec)->vector_capacity < mem_needed) { \
      cscript_memsize iterator_index = cast(cscript_memsize, cast(element_type*, *(iterator)) - cast(element_type*, (vec)->vector_ptr)); \
      cscript_reallocvector(ctxt, (vec)->vector_ptr, (vec)->vector_capacity, mem_needed, element_type); \
      *(iterator) = (cast(element_type*, (vec)->vector_ptr) + iterator_index); \
      (vec)->vector_capacity = mem_needed; } \
    element_type* it_end_vinsert = (cast(element_type*, (vec)->vector_ptr) + (vec)->vector_size); \
    (vec)->vector_size += range_size; \
    memmove(cast(element_type*, *(iterator))+range_size, cast(element_type*, *(iterator)), (it_end_vinsert - cast(element_type*, *(iterator)))*sizeof(element_type)); \
    memcpy(*(iterator), *(range_it_begin), range_size*sizeof(element_type)); }
  
#define cscript_vector_insert_element(ctxt, vec, iterator, element_value, element_type) \
  { cscript_memsize mem_needed = (vec)->vector_size + 1; \
    if ((vec)->vector_capacity < mem_needed) { \
      cscript_memsize iterator_index = cast(cscript_memsize, cast(element_type*, *(iterator)) - cast(element_type*, (vec)->vector_ptr)); \
      cscript_reallocvector(ctxt, (vec)->vector_ptr, (vec)->vector_capacity, mem_needed, element_type); \
      *(iterator) = (cast(element_type*, (vec)->vector_ptr) + iterator_index); \
      (vec)->vector_capacity = mem_needed; } \
    element_type* it_end_vinsert = (cast(element_type*, (vec)->vector_ptr) + (vec)->vector_size); \
    (vec)->vector_size += 1; \
    memmove(cast(element_type*, *(iterator))+1, cast(element_type*, *(iterator)), (it_end_vinsert - cast(element_type*, *(iterator)))*sizeof(element_type)); \
    **(iterator) = element_value;  }

#endif //CSCRIPT_VECTOR_H