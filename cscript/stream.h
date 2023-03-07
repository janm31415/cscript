#ifndef CSCRIPT_STREAM_H
#define CSCRIPT_STREAM_H

#include "cscript.h"
#include "memory.h"

/* end of stream */
#define END_OF_STREAM (cast(cscript_memsize, -1))	

/*
* Struct that represents a stream of bytes, that you can read
* from and write to.
*
* Writing is guaranteed to succeed as long as there is enough
* memory left on the machine.
* Reading, in turn, can happen to only be executed partly,
* which depends on how much reading content there is in the stream.
*/
typedef struct cscript_stream
  {
  char* data; // The data in memory
  cscript_memsize position; // The current read/write position
  cscript_memsize length; // The number of bytes written to the stream
  cscript_memsize capacity; // The actual number of bytes allocated
  } cscript_stream;

/*
* Initializes stream and reserves capac number of bytes.
*/
void cscript_stream_init(cscript_context* ctxt, cscript_stream* stream, cscript_memsize capac);

/*
* Clears all resources used by a stream.
* Make sure to call this on every stream when you are done with it.
*/
void cscript_stream_close(cscript_context* ctxt, cscript_stream* stream);

/*
* Reserves a given capacity for the stream
*/
void cscript_stream_reserve(cscript_context* ctxt, cscript_stream* stream, cscript_memsize size);

/*
* Writes count bytes from buffer to stream at the current position.
* offset determines the offset of the input buffer.
* Returns the total number of bytes written to stream.
*
* The difference to stream_insert is, that this function overwrites the data
* of the stream it collides with from position to end.
*/
cscript_memsize cscript_stream_write(cscript_context* ctxt, cscript_stream* stream, const char* buffer, cscript_memsize count, cscript_memsize offset);

/*
* Writes a byte to stream.
*/
void cscript_stream_write_byte(cscript_context* ctxt, cscript_stream* stream, char value);

/*
* Writes a string to stream.
*/
cscript_memsize cscript_stream_write_str(cscript_context* ctxt, cscript_stream* stream, const char* str);

/*
* Inserts count bytes from buffer into the stream at it's current position.
* offset determines the offset of the input buffer.
* Returns the total number of bytes written to stream.
*
* The difference to stream_write is, that this function does not overwrite the
* data is collides with, instead it actually inserts it into memory, which
* is obviously more expensive.
*/
cscript_memsize cscript_stream_insert(cscript_context* ctxt, cscript_stream* stream, const char* buffer, cscript_memsize count, cscript_memsize offset);

/*
* Inserts a byte into stream.
*/
void cscript_stream_insert_byte(cscript_context* ctxt, cscript_stream* stream, char value);

/*
* Inserts a string into stream.
*/
cscript_memsize cscript_stream_insert_str(cscript_context* ctxt, cscript_stream* stream, const char* str);

/*
* Reads count bytes from stream into buffer.
* offset determines the offset of the output buffer.
* Returns the actual number of bytes read or END_OF_STREAM if the end was reached.
*/
cscript_memsize cscript_stream_read(cscript_stream* stream, char* buffer, cscript_memsize count, cscript_memsize offset);

/*
* Reads a byte from stream.
* Returns the integer value of the byte or END_OF_STREAM
* if the end was reached.
*/
cscript_memsize cscript_stream_read_byte(cscript_stream* stream);

/*
* Sets all bytes, position and the length to zero.
* The actual memory is kept allocated. Capacity stays the same.
* If your intention is to free the memory, use stream_close.
*/
void cscript_stream_clear(cscript_stream* stream);

/*
* Copies the whole content of one stream to another at the
* destinations position.
* Returns 1 if success or 0 if failure.
*/
cscript_memsize cscript_stream_copy(cscript_context* ctxt, cscript_stream* src, cscript_stream* dest);

/*
* Sets the position of stream.
* Returns the new position of the stream or END_OF_STREAM on failure.
*/
cscript_memsize cscript_stream_set_pos(cscript_stream* stream, cscript_memsize pos);

/*
* Sets the position of stream.
* Returns the new position of the stream or END_OF_STREAM on failure.
*/
cscript_memsize cscript_stream_seek(cscript_stream* stream, cscript_memsize pos);

/*
* Sets the position of stream to 0.
* Returns the new position of the stream or END_OF_STREAM on failure.
*/
cscript_memsize cscript_stream_rewind(cscript_stream* stream);

#endif //CSCRIPT_STREAM_H