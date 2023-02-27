#include "token_tests.h"

#include "cscript/token.h"
#include "cscript/context.h"
#include "cscript/stream.h"

#include "test_assert.h"

#include <string.h>

static void test_number_recognition()
  {
  cscript_flonum fl = cscript_to_flonum("1.2");
  TEST_EQ_DOUBLE(1.2, fl);
  fl = cscript_to_flonum("3.14159");
  TEST_EQ_DOUBLE(3.14159, fl);
  fl = cscript_to_flonum("-3.14159");
  TEST_EQ_DOUBLE(-3.14159, fl);
  fl = cscript_to_flonum("10");
  TEST_EQ_DOUBLE(10.0, fl);

  int is_real;
  int is_scientific;
  TEST_EQ_INT(1, cscript_is_number(&is_real, &is_scientific, "1"));
  TEST_EQ_INT(0, is_real);
  TEST_EQ_INT(0, is_scientific);
  TEST_EQ_INT(1, cscript_is_number(&is_real, &is_scientific, "1.0"));
  TEST_EQ_INT(1, is_real);
  TEST_EQ_INT(0, is_scientific);
  TEST_EQ_INT(1, cscript_is_number(&is_real, &is_scientific, "1e+0"));
  TEST_EQ_INT(1, is_real);
  TEST_EQ_INT(1, is_scientific);
  }

static void tokenize_list()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector tokens = cscript_script2tokens(ctxt, "(list a b)");
  TEST_EQ_INT(5, tokens.vector_size);  
  TEST_EQ_INT(CSCRIPT_T_LEFT_ROUND_BRACKET, cscript_vector_at(&tokens, 0, token)->type);
  TEST_EQ_INT(CSCRIPT_T_ID, cscript_vector_at(&tokens, 1, token)->type);
  TEST_EQ_INT(CSCRIPT_T_ID, cscript_vector_at(&tokens, 2, token)->type);
  TEST_EQ_INT(CSCRIPT_T_ID, cscript_vector_at(&tokens, 3, token)->type);
  TEST_EQ_INT(CSCRIPT_T_RIGHT_ROUND_BRACKET, cscript_vector_at(&tokens, 4, token)->type);
  destroy_tokens_vector(ctxt, &tokens);
  cscript_close(ctxt);
  }

static void tokenize_fixnum_real()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector tokens = cscript_script2tokens(ctxt, "(+ 1 2.0)");
  TEST_EQ_INT(5, tokens.vector_size);
  TEST_EQ_INT(CSCRIPT_T_LEFT_ROUND_BRACKET, cscript_vector_at(&tokens, 0, token)->type);
  TEST_EQ_INT(CSCRIPT_T_PLUS, cscript_vector_at(&tokens, 1, token)->type);
  TEST_EQ_INT(CSCRIPT_T_FIXNUM, cscript_vector_at(&tokens, 2, token)->type);
  TEST_EQ_INT(CSCRIPT_T_FLONUM, cscript_vector_at(&tokens, 3, token)->type);
  TEST_EQ_INT(CSCRIPT_T_RIGHT_ROUND_BRACKET, cscript_vector_at(&tokens, 4, token)->type);
  destroy_tokens_vector(ctxt, &tokens);
  cscript_close(ctxt);
  }


void run_all_token_tests()
  {
  test_number_recognition();
  tokenize_list();
  tokenize_fixnum_real();
  }