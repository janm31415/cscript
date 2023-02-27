#include "stream.h"

#include <string.h>
#include <stdlib.h>

#define write_space_left(stream) \
  (stream->capacity - stream->position)

#define read_space_left(stream) \
  (stream->length - stream->position)

#define capacity_left(stream) \
  (stream->capacity - stream->length)

static void cscript_stream_resize(cscript_stream* stream, cscript_memsize size);

void cscript_stream_init(cscript_context* ctxt, cscript_stream* stream, cscript_memsize capac)
  {
  stream->data = cast(char*, cscript_malloc(ctxt, capac));  
  stream->length = 0;
  stream->position = 0;
  stream->capacity = capac;
  }

void cscript_stream_close(cscript_context* ctxt, cscript_stream* stream)
  {
  cscript_free(ctxt, stream->data, stream->capacity);  
  stream->data = NULL;
  stream->capacity = 0;
  stream->length = 0;
  stream->position = 0;
  }

void cscript_stream_reserve(cscript_context* ctxt, cscript_stream* stream, cscript_memsize size)
  {
  if (stream->capacity < size)
    {
    stream->data = cscript_realloc(ctxt, stream->data, stream->capacity, size);
    stream->capacity = size;
    } 
  }

cscript_memsize cscript_stream_write(cscript_context* ctxt, cscript_stream* stream, const char* buffer, cscript_memsize count, cscript_memsize offset)
  {
  cscript_memsize space_left = write_space_left(stream);
  if (space_left < count) 
    {
    cscript_stream_reserve(ctxt, stream, stream->capacity + count-space_left);    
    }

  char* dest = &(stream->data[stream->position]);
  const char* source = &buffer[offset];
  memcpy(dest, source, count);
  stream->position += count;
  stream->length += count;
  return count;  
  }

void cscript_stream_write_byte(cscript_context* ctxt, cscript_stream* stream, char value)
  {
  cscript_stream_write(ctxt, stream, &value, 1, 0);
  }

cscript_memsize cscript_stream_write_str(cscript_context* ctxt, cscript_stream* stream, const char* str)
  {
  cscript_memsize len = cast(cscript_memsize, strlen(str));
  return cscript_stream_write(ctxt, stream, str, len, 0);
  }

cscript_memsize cscript_stream_insert(cscript_context* ctxt, cscript_stream* stream, const char* buffer, cscript_memsize count, cscript_memsize offset)
  {
  cscript_memsize space_left = capacity_left(stream);
  if (space_left < count) 
    {
    cscript_stream_reserve(ctxt, stream, stream->capacity + count - space_left);    
    }

  char* src = &(stream->data[stream->position]);
  char* move_dest = &(stream->data[stream->position + count]);
  cscript_memsize read_spc = read_space_left(stream);

  // Move data
  memmove(move_dest, src, read_spc);

  // Insert input buffer at pos
  memcpy(src, &buffer[offset], count);

  // Update position and length
  stream->position += count;
  stream->length += count;
  return count;
  }

void cscript_stream_insert_byte(cscript_context* ctxt, cscript_stream* stream, char value)
  {
  cscript_stream_insert(ctxt, stream, &value, 1, 0);
  }

cscript_memsize cscript_stream_insert_str(cscript_context* ctxt, cscript_stream* stream, const char* str)
  {  
  cscript_memsize len = cast(cscript_memsize, strlen(str) + 1);
  return cscript_stream_insert(ctxt, stream, str, len, 0);
  }

cscript_memsize cscript_stream_read(cscript_stream* stream, char* buffer, cscript_memsize count, cscript_memsize offset)
  {
  cscript_memsize space_left = read_space_left(stream);
  if (space_left == 0) 
    {
    return END_OF_STREAM;
    }
  if (space_left > count) 
    {
    space_left = count;
    }

  char* dest = &buffer[offset];
  char* source = &(stream->data[stream->position]);

  memcpy(dest, source, space_left);
  stream->position += space_left;

  return space_left;
  }

cscript_memsize cscript_stream_read_byte(cscript_stream* stream)
  {
  cscript_memsize space_left = read_space_left(stream);
  if (space_left == 0) 
    {
    return END_OF_STREAM;
    }

  return cast(cscript_memsize, stream->data[stream->position++]);
  }

void cscript_stream_clear(cscript_stream* stream)
  {
  memset(stream->data, 0, stream->capacity);
  stream->length = 0;
  stream->position = 0;
  }

cscript_memsize cscript_stream_copy(cscript_context* ctxt, cscript_stream* src, cscript_stream* dest)
  {
  return cast(cscript_memsize, cscript_stream_write(ctxt, dest, src->data, src->length, 0) == src->length);
  }

cscript_memsize cscript_stream_set_pos(cscript_stream* stream, cscript_memsize pos)
  {
  if (pos < 0 || pos > stream->length) 
    {
    return END_OF_STREAM;
    }

  stream->position = pos;
  return stream->position;
  }

cscript_memsize cscript_stream_seek(cscript_stream* stream, cscript_memsize pos)
  {
  return cscript_stream_set_pos(stream, pos);
  }

cscript_memsize cscript_stream_rewind(cscript_stream* stream)
  {
  return cscript_stream_set_pos(stream, 0);
  }