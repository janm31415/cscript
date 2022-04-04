///////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////

#include "parsetests.h"

#include "cscript/tokenize.h"
#include "cscript/parse.h"

#include <iomanip>
#include <iostream>
#include <fstream>

#include "test_assert.h"

COMPILER_BEGIN

namespace
  {
  void test_parse_expression()
    {
    auto tokens = tokenize("3.0*(2/0.5-7);");
    TEST_EQ(int(10), int(tokens.size()));
    auto prog = make_program(tokens);
    TEST_EQ(int(1), (int)prog.statements.size());
    }

  void test_parse_negative()
    {
    auto tokens = tokenize("-5;");
    TEST_EQ(int(3), int(tokens.size()));
    auto prog = make_program(tokens);
    TEST_EQ(int(1), (int)prog.statements.size());
    }

  void test_parse_parameters()
    {
    auto tokens = tokenize("(float f, int* i) i = f;");
    auto prog = make_program(tokens);
    TEST_EQ(int(2), (int)prog.parameters.size());
    }

  void test_parse_for()
    {
    auto tokens = tokenize("for (int i = 0; i < 50; i += 1) { float j = i; }");
    auto prog = make_program(tokens);
    TEST_EQ(int(1), (int)prog.statements.size());
    }

  void test_parse_harmonic()
    {
    auto tokens = tokenize("float sum = 0.0; for (int i = 1; i < 50; i += 1) { sum += 1.0/i; } sum;");
    auto prog = make_program(tokens);
    TEST_EQ(int(3), (int)prog.statements.size());
    TEST_ASSERT(std::holds_alternative<Float>(prog.statements[0]));
    TEST_ASSERT(std::holds_alternative<For>(prog.statements[1]));
    TEST_ASSERT(std::holds_alternative<Expression>(prog.statements[2]));
    }

  void test_parse_inc()
    {
    auto tokens = tokenize("int i = 3; (++i);");
    auto prog = make_program(tokens);
    TEST_EQ(int(2), (int)prog.statements.size());
    }
  }

COMPILER_END

void run_all_parse_tests()
  {
  using namespace COMPILER;
  test_parse_expression();
  test_parse_negative();
  test_parse_parameters();
  test_parse_for();
  test_parse_harmonic();
  test_parse_inc();
  }