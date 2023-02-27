#include "string_tests.h"
#include "test_assert.h"

#include "cscript/cscript.h"
#include "cscript/string.h"
#include "cscript/context.h"

static void test_string_init()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_string s;
  cscript_string_init(ctxt, &s, "Hello world!");
  TEST_EQ_INT(12, s.string_length);
  TEST_EQ_INT(13, s.string_capacity);
  cscript_string_destroy(ctxt, &s);
  cscript_close(ctxt);
  }

static void test_string_functions()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_string s;
  cscript_string_init(ctxt, &s, "Hello world!");
  
  TEST_EQ_INT('H', *cscript_string_begin(&s));
  TEST_EQ_INT('!', *cscript_string_back(&s));
  TEST_EQ_INT(0, *cscript_string_end(&s));
  TEST_EQ_INT('o', *cscript_string_at(&s, 4));

  *cscript_string_back(&s) = '?';
  TEST_EQ_INT('?', *cscript_string_back(&s));
  cscript_string_destroy(ctxt, &s);
  cscript_close(ctxt);
  }

static void test_string_push_back()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_string s;
  cscript_string_init(ctxt, &s, "Hello world!");
  cscript_string_push_back(ctxt, &s, '!');
  cscript_string_push_back(ctxt, &s, '!');
  TEST_EQ_INT(14, s.string_length);
  TEST_EQ_INT(26, s.string_capacity);
  cscript_string_pop_back(&s);
  TEST_EQ_INT(13, s.string_length);
  TEST_EQ_INT(26, s.string_capacity);
  cscript_string_destroy(ctxt, &s);
  cscript_close(ctxt);
  }

static void test_string_append()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_string s1;
  cscript_string_init(ctxt, &s1, "Hello ");
  TEST_EQ_INT(6, s1.string_length);
  cscript_string s2;
  cscript_string_init(ctxt, &s2, "world!");
  cscript_string_append(ctxt, &s1, &s2);
  TEST_EQ_INT(12, s1.string_length);
  TEST_EQ_INT('H', *cscript_string_begin(&s1));
  TEST_EQ_INT('!', *cscript_string_back(&s1));
  TEST_EQ_INT(0, *cscript_string_end(&s1));
  TEST_EQ_INT('o', *cscript_string_at(&s1, 4));
  cscript_string_destroy(ctxt, &s1);
  cscript_string_destroy(ctxt, &s2);
  cscript_close(ctxt);
  }

static void test_string_append_2()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_string s1;
  cscript_string_init(ctxt, &s1, "Hello ");
  TEST_EQ_INT(6, s1.string_length);
  cscript_string_append_cstr(ctxt, &s1, "world!");
  TEST_EQ_INT(12, s1.string_length);
  TEST_EQ_INT('H', *cscript_string_begin(&s1));
  TEST_EQ_INT('!', *cscript_string_back(&s1));
  TEST_EQ_INT(0, *cscript_string_end(&s1));
  TEST_EQ_INT('o', *cscript_string_at(&s1, 4));
  cscript_string_destroy(ctxt, &s1);
  cscript_close(ctxt);
  }

static void test_string_compare_less_aux(const char* left, const char* right, int expected)
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_string s1, s2;
  cscript_string_init(ctxt, &s1, left);
  cscript_string_init(ctxt, &s2, right);
  int compare_result = cscript_string_compare_less(&s1, &s2);
  TEST_EQ_INT(expected, compare_result);
  cscript_string_destroy(ctxt, &s1);
  cscript_string_destroy(ctxt, &s2);
  cscript_close(ctxt);
  }

static void test_string_compare()
  {
  test_string_compare_less_aux("a", "b", 1);
  test_string_compare_less_aux("b", "a", 0);
  test_string_compare_less_aux("a", "a", 0);
  test_string_compare_less_aux("brabbelauto", "brabbelbox", 1);
  test_string_compare_less_aux("brabbelbox", "brabbelauto", 0);
  test_string_compare_less_aux("brabbelauto", "brabbelauto", 0);
  }

void run_all_string_tests()
  {
  test_string_init();
  test_string_functions();
  test_string_push_back();
  test_string_append();
  test_string_append_2();
  test_string_compare();
  }