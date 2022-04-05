///////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////

#include "cscript/optimize.h"
#include "cscript/parse.h"
#include "cscript/tokenize.h"
#include "cscript/utility.h"
#include "cscript/visitor.h"
#include "cscript/pp_visitor.h"
#include "cscript/compiler.h"
#include "cscript/peephole.h"

#include <iomanip>
#include <iostream>
#include <fstream>
#include <sstream>

#include "test_assert.h"

COMPILER_BEGIN


void test_visitor_print()
  {
  pretty_print_visitor ppv;
  auto tokens = tokenize("for (int i = 0; i < 50; ++i) {3.0*(2/0.5-7);}");
  auto prog = make_program(tokens);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  }


void test_visitor_print2()
  {
  pretty_print_visitor ppv;
  auto tokens = tokenize("3*5*4;");
  auto prog = make_program(tokens);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  }


void test_optimize_harmonic()
  {
  pretty_print_visitor ppv;
  auto tokens = tokenize("float sum = 0.0; for (int i = 1; i < 50; i += 1) { sum += 1.0/i; } sum;");
  auto prog = make_program(tokens);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  optimize(prog);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  VM::vmcode code;
  compile(code, prog);
  peephole_optimization(code);
  code.stream(std::cout);
  }

void test_optimize_fibonacci()
  {
  pretty_print_visitor ppv;
  auto tokens = tokenize("(int i) int a = 0; int b = 1; for (int j = 0; j < i; ++j) { int c = a+b; a = b; b = c; } a;");
  auto prog = make_program(tokens);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  optimize(prog);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  VM::vmcode code;
  compile(code, prog);
  peephole_optimization(code);
  code.stream(std::cout);
  }

void test_optimize_hamming()
  {
  pretty_print_visitor ppv;
  auto tokens = tokenize("(int* a, int* b, int size) int hamming = 0; for (int i = 0; i < size; ++i) { hamming += a[i] != b[i];} hamming;");
  auto prog = make_program(tokens);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  optimize(prog);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  VM::vmcode code;
  compile(code, prog);
  peephole_optimization(code);
  code.stream(std::cout);
  }

COMPILER_END

void run_all_optimize_tests()
  {
  using namespace COMPILER;
  //test_visitor_print();
  //test_visitor_print2();
  //test_optimize_harmonic();
  //test_optimize_fibonacci();
  test_optimize_hamming();
  }