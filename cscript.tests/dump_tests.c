#include "dump_tests.h"
#include "test_assert.h"

#include "cscript/token.h"
#include "cscript/context.h"
#include "cscript/dump.h"
#include "cscript/parser.h"

static void test_dump(const char* script)
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector tokens = cscript_script2tokens(ctxt, script);
  cscript_program prog = make_program(ctxt, &tokens);
  cscript_dump_visitor* dumper = cscript_dump_visitor_new(ctxt);
  cscript_visit_program(ctxt, dumper->visitor, &prog);
  int compare = strcmp(dumper->s.string_ptr, script);
  TEST_EQ_INT(0, compare);
  if (compare != 0)
    {
    printf("Expected: %s\nActual: %s\n", script, dumper->s.string_ptr);
    }
  cscript_dump_visitor_free(ctxt, dumper);
  destroy_tokens_vector(ctxt, &tokens);
  cscript_program_destroy(ctxt, &prog);
  cscript_close(ctxt);
  }

void run_all_dump_tests()
  {
  test_dump(";\n");
  test_dump("3;\n");
  test_dump("3.140000;\n");
  test_dump("int i;\n");
  test_dump("float f;\n");
  test_dump("(1+2+3.140000)*27<3.140000-5%3;\n");
  }