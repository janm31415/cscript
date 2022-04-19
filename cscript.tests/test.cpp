#include "test_assert.h"

#include "tokenizetests.h"
#include "parsetests.h"
#include "optimizetests.h"
#include "compiletests.h"
#include "cscripttests.h"

#include <ctime>

int main(int /*argc*/, const char* /*argv*/[])
  {
  InitTestEngine();

  auto tic = std::clock();
  //run_all_tokenize_tests();
  //run_all_parse_tests();
  //run_all_optimize_tests();
  //run_all_compile_tests();
  run_all_cscript_tests();
  auto toc = std::clock();

  if (!testing_fails) 
    {
    TEST_OUTPUT_LINE("Succes: %d tests passed.", testing_success);
    }
  else 
    {
    TEST_OUTPUT_LINE("FAILURE: %d out of %d tests failed (%d failures).", testing_fails, testing_success+testing_fails, testing_fails);
    }
  TEST_OUTPUT_LINE("Test time: %f seconds.", (double)(toc - tic)/(double)CLOCKS_PER_SEC);
  return CloseTestEngine(true);
  }
