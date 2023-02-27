#include "stream_tests.h"
#include "test_assert.h"

#include "cscript/stream.h"
#include "cscript/context.h"

static void test_stream_construction()
  {
  cscript_stream str;
  cscript_context* ctxt = cscript_open(256);
  cscript_stream_init(ctxt, &str, 10);
  TEST_EQ_INT(10, str.capacity);
  TEST_EQ_INT(0, str.position);
  TEST_EQ_INT(0, str.length);
  cscript_stream_close(ctxt, &str);
  cscript_close(ctxt);
  }

static void test_stream_write_read()
  {
  cscript_stream str;
  cscript_context* ctxt = cscript_open(256);
  cscript_stream_init(ctxt, &str, 0);
  TEST_EQ_INT(0, str.capacity);
  TEST_EQ_INT(0, str.position);
  TEST_EQ_INT(0, str.length);
  cscript_stream_write(ctxt, &str, "Hello world!", 12, 0);
  TEST_EQ_INT(12, str.capacity);
  TEST_EQ_INT(12, str.position);
  TEST_EQ_INT(12, str.length);
  char buffer[256];
  cscript_memsize bytes_read = cscript_stream_read(&str, buffer, 12, 0);
  TEST_EQ_INT(END_OF_STREAM, bytes_read);
  cscript_stream_rewind(&str);
  bytes_read = cscript_stream_read(&str, buffer, 12, 0);
  TEST_EQ_INT(12, bytes_read);
  TEST_EQ_INT('H', buffer[0]);
  TEST_EQ_INT('e', buffer[1]);
  TEST_EQ_INT('l', buffer[2]);
  TEST_EQ_INT('l', buffer[3]);
  TEST_EQ_INT('o', buffer[4]);
  cscript_stream_close(ctxt, &str);
  cscript_close(ctxt);
  }

void run_all_stream_tests()
  {
  test_stream_construction();
  test_stream_write_read();
  }