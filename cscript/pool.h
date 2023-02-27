#ifndef CSCRIPT_POOL_H
#define CSCRIPT_POOL_H

#include "cscript.h"
#include "object.h"
#include "vector.h"

typedef struct cscript_chunk cscript_chunk;

typedef struct cscript_chunk
  {
  /*
  * When a chunk is free, the `next` contains the
  * address of the next chunk in a list.
  *
  * When it's allocated, this space is used by
  * the user.
  */
  cscript_chunk* next;
  } cscript_chunk;

typedef struct cscript_pool_allocator
  {
  cscript_memsize chunks_per_block;
  cscript_memsize chunk_size;
  cscript_chunk* alloc; // allocation pointer
  cscript_vector block_addresses; // for clean up at the end
  } cscript_pool_allocator;

void cscript_pool_allocator_init(cscript_context* ctxt, cscript_pool_allocator* pool, cscript_memsize chunks_per_block, cscript_memsize chunk_size);
void cscript_pool_allocator_destroy(cscript_context* ctxt, cscript_pool_allocator* pool);

void* cscript_pool_allocate(cscript_context* ctxt, cscript_pool_allocator* pool);
void cscript_pool_deallocate(cscript_pool_allocator* pool, void* chunk);

#endif //CSCRIPT_POOL_H