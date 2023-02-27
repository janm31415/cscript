#include "pool.h"
#include "context.h"
#include "memory.h"

void cscript_pool_allocator_init(cscript_context* ctxt, cscript_pool_allocator* pool, cscript_memsize chunks_per_block, cscript_memsize chunk_size)
  {
  pool->alloc = NULL;
  pool->chunks_per_block = chunks_per_block;
  pool->chunk_size = chunk_size;
  cscript_vector_init(ctxt, &pool->block_addresses, void*);
  }

void cscript_pool_allocator_destroy(cscript_context* ctxt, cscript_pool_allocator* pool)
  {
  const cscript_memsize block_size = pool->chunks_per_block * pool->chunk_size;
  void** it = cscript_vector_begin(&pool->block_addresses, void*);
  void** it_end = cscript_vector_end(&pool->block_addresses, void*);
  for (; it != it_end; ++it)
    {
    cscript_free(ctxt, *it, block_size);
    }
  cscript_vector_destroy(ctxt, &pool->block_addresses);
  }

static cscript_chunk* cscript_pool_allocate_block(cscript_context* ctxt, cscript_pool_allocator* pool)
  {
  const cscript_memsize block_size = pool->chunks_per_block * pool->chunk_size;
  void* new_memory = cscript_malloc(ctxt, block_size);
  // store the new memory in our block addresses list, so that we can free them when cscript_pool_allocator_destroy is called
  cscript_vector_push_back(ctxt, &pool->block_addresses, new_memory, void*);
  cscript_chunk* block = cast(cscript_chunk*, new_memory);  
  cscript_chunk* chunk = block;
  // chain all the chunks in the block
  for (cscript_memsize i = 1; i < pool->chunks_per_block; ++i)
    {
    chunk->next = cast(cscript_chunk*, cast(char*, chunk) + pool->chunk_size);
    chunk = chunk->next;
    }
  chunk->next = NULL;  
  return block;
  }

void* cscript_pool_allocate(cscript_context* ctxt, cscript_pool_allocator* pool)
  {
  if (pool->alloc == NULL) // no free chunks left. Allocate a new block.
    {
    pool->alloc = cscript_pool_allocate_block(ctxt, pool);
    }
  cscript_chunk* free_chunk = pool->alloc; // first free chunk
  pool->alloc = pool->alloc->next; // point to the next free chunk
  return free_chunk;
  }

void cscript_pool_deallocate(cscript_pool_allocator* pool, void* chunk)
  {
  // The freed chunk's next pointer points to the current allocation pointer
  cast(cscript_chunk*, chunk)->next = pool->alloc;
  // And the allocation pointer is now set to the returned (free) chunk
  pool->alloc = cast(cscript_chunk*, chunk);
  }