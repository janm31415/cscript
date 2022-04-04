///////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////

#include "tokenizetests.h"

#include <iomanip>
#include <iostream>
#include <fstream>

#include "cscript/tokenize.h"

#include "test_assert.h"

COMPILER_BEGIN

namespace
  {
  void test_tokenize_1()
    {
    auto tokens = tokenize("3+2.1");
    TEST_EQ(int(3), int(tokens.size()));
    TEST_EQ(token::T_INTEGER, tokens[0].type);
    TEST_EQ(token::T_PLUS, tokens[1].type);
    TEST_EQ(token::T_REAL, tokens[2].type);
    }

  void test_tokenize_2()
    {
    auto tokens = tokenize("3+2.1f");
    TEST_EQ(int(3), int(tokens.size()));
    TEST_EQ(token::T_INTEGER, tokens[0].type);
    TEST_EQ(token::T_PLUS, tokens[1].type);
    TEST_EQ(token::T_REAL, tokens[2].type);

    bool is_real;
    TEST_ASSERT(is_number(is_real, "1")); TEST_ASSERT(!is_real);
    TEST_ASSERT(is_number(is_real, "5")); TEST_ASSERT(!is_real);
    TEST_ASSERT(is_number(is_real, "1351.1325")); TEST_ASSERT(is_real); TEST_ASSERT(to_double("1351.1325") == 1351.1325);
    TEST_ASSERT(is_number(is_real, "13513.353f")); TEST_ASSERT(is_real); TEST_ASSERT(to_double("13513.353f") == 13513.353);
    TEST_ASSERT(is_number(is_real, "1.f")); TEST_ASSERT(is_real); TEST_ASSERT(to_double("1.f") == 1.0);
    TEST_ASSERT(!is_number(is_real, "1f"));
    TEST_ASSERT(!is_number(is_real, "f"));
    TEST_ASSERT(!is_number(is_real, "Hello.World"));
    TEST_ASSERT(is_number(is_real, ".f"));  TEST_ASSERT(is_real); TEST_ASSERT(to_double(".f") == 0.0);
    }

  }

COMPILER_END

void run_all_tokenize_tests()
  {
  using namespace COMPILER;
  test_tokenize_1();
  test_tokenize_2();
  }