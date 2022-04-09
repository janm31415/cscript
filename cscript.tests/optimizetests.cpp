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

#include "vm/vm.h"

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

void test_optimize_qsort()
  {
  pretty_print_visitor ppv;
  std::string sourcecode = R"((int* a, int* stack, int size)
      int lo = 0;
      int hi = size-1;
      // initialize top of stack
      int top = -1;
      // push initial values of l and h to stack
      stack[++top] = lo;
      stack[++top] = hi;
      for (; top >= 0; )
        {
        hi = stack[top];
        --top;
        lo = stack[top];
        --top;
        // partitioning algorithm
        // Set pivot element at its correct position
        // in sorted array
        int x = a[hi];
        int i = (lo - 1);
        for (int j = lo; j <= hi - 1; ++j)
          {
          if (a[j] <= x)
            {
            ++i;
            int tmp = a[i];
            a[i] = a[j];
            a[j] = tmp;
            }
          }
        int tmp2 = a[i+1];
        a[i+1] = a[hi];
        a[hi] = tmp2;
        int p = i+1;
        // end partitioning algorithm

        // If there are elements on left side of pivot,
        // then push left side to stack
        if (p - 1 > lo)
          {
          stack[++top] = lo;
          stack[++top] = p-1;
          }

        // If there are elements on right side of pivot,
        // then push right side to stack
        if (p + 1 < hi)
          {
          stack[++top] = p+1;
          stack[++top] = hi;
          }
        }
)";
  auto tokens = tokenize(sourcecode);
  auto prog = make_program(tokens);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  optimize(prog);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  VM::vmcode code;
  compile(code, prog);
  peephole_optimization(code);
  code.stream(std::cout);
  uint64_t size;
  uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
  std::cout << "Byte size: " << size << "\n";
  VM::free_bytecode(f, size);
  }


void test_optimize_qsort_double()
  {
  pretty_print_visitor ppv;
  std::string sourcecode = R"((float* a, int* stack, int size)
      int lo = 0;
      int hi = size-1;
      // initialize top of stack
      int top = -1;
      // push initial values of l and h to stack
      stack[++top] = lo;
      stack[++top] = hi;
      while (top >= 0)
        {
        hi = stack[top];
        --top;
        lo = stack[top];
        --top;
        // partitioning algorithm
        // Set pivot element at its correct position
        // in sorted array
        float x = a[hi];
        int i = (lo - 1);
        for (int j = lo; j <= hi - 1; ++j)
          {
          if (a[j] <= x)
            {
            ++i;
            float tmp = a[i];
            a[i] = a[j];
            a[j] = tmp;
            }
          }
        float tmp2 = a[i+1];
        a[i+1] = a[hi];
        a[hi] = tmp2;
        int p = i+1;
        // end partitioning algorithm

        // If there are elements on left side of pivot,
        // then push left side to stack
        if (p - 1 > lo)
          {
          stack[++top] = lo;
          stack[++top] = p-1;
          }

        // If there are elements on right side of pivot,
        // then push right side to stack
        if (p + 1 < hi)
          {
          stack[++top] = p+1;
          stack[++top] = hi;
          }
        }
)";
  auto tokens = tokenize(sourcecode);
  auto prog = make_program(tokens);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  optimize(prog);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  VM::vmcode code;
  compile(code, prog);
  peephole_optimization(code);
  code.stream(std::cout);
  uint64_t size;
  uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
  std::cout << "Byte size: " << size << "\n";
  VM::free_bytecode(f, size);
  }

void test_optimize_add_zero()
  {
  pretty_print_visitor ppv;
  auto tokens = tokenize("(int a) a += 0; a;");
  auto prog = make_program(tokens);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  optimize(prog);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  VM::vmcode code;
  compile(code, prog);
  peephole_optimization(code);
  code.stream(std::cout);
  }

void test_optimize_mul_zero()
  {
  pretty_print_visitor ppv;
  auto tokens = tokenize("(int a) a *= 0; a;");
  auto prog = make_program(tokens);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  optimize(prog);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  VM::vmcode code;
  compile(code, prog);
  peephole_optimization(code);
  code.stream(std::cout);
  }

void test_optimize_div_one()
  {
  pretty_print_visitor ppv;
  auto tokens = tokenize("(int a) a /= 1; a;");
  auto prog = make_program(tokens);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  optimize(prog);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  VM::vmcode code;
  compile(code, prog);
  peephole_optimization(code);
  code.stream(std::cout);
  }

void test_optimize_constant_variables()
  {
  pretty_print_visitor ppv;
  auto tokens = tokenize("() int x = 1; int y = 2; x+y;");
  auto prog = make_program(tokens);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  optimize(prog);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  VM::vmcode code;
  compile(code, prog);
  peephole_optimization(code);
  code.stream(std::cout);
  }

void test_optimize_unused_variables()
  {
  pretty_print_visitor ppv;
  auto tokens = tokenize("() int x = 1, y = 2; float r, g, b, z;x;");
  auto prog = make_program(tokens);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  optimize(prog);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  VM::vmcode code;
  compile(code, prog);
  peephole_optimization(code);
  code.stream(std::cout);
  }

void test_optimize_number_transition()
  {
  pretty_print_visitor ppv;
  //auto tokens = tokenize("() float f; int i; f = 3.1415; i = 1; i += f; i;");
  auto tokens = tokenize("(int i) int a = 0; int b = 1; for (int j = 0; j < i; ++j) { int c = a+b; a = b; b = c; } a;");
  auto prog = make_program(tokens);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  //optimize(prog);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  VM::vmcode code;
  compile(code, prog);
  peephole_optimization(code);
  uint64_t size;
  uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
  VM::registers regs;
  VM::run_bytecode(f, size, regs);
  //std::cout << "Byte size: " << size << "\n";
  VM::free_bytecode(f, size);
  code.stream(std::cout);
  }

void test_optimize_super_operators()
  {
  pretty_print_visitor ppv;
  //auto tokens = tokenize("() float f; int i; f = 3.1415; i = 1; i += f; i;");
  auto tokens = tokenize("()110+25;");
  auto prog = make_program(tokens);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  //optimize(prog);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  VM::vmcode code;
  compile(code, prog);
  peephole_optimization(code);
  uint64_t size;
  uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
  VM::registers regs;
  VM::run_bytecode(f, size, regs);
  //std::cout << "Byte size: " << size << "\n";
  VM::free_bytecode(f, size);
  code.stream(std::cout);
  }

void test_optimize_digitspi()
  {
  pretty_print_visitor ppv;
  std::string script = R"((int nr_of_terms)
/* This method computes pi by using the power series expansion of 
   atan(x) = x - x^3/3 + x^5/5 - ... together with formulas like 
   pi = 16*atan(1/5) - 4*atan(1/239). 
   This gives about 1.4 decimals per term.
*/
    float x1 = 1.0/5.0;
    float x2 = 1.0/239.0;
    float pi = 0;
    for (int i = 0; i < nr_of_terms; ++i)
      {
      float sign = pow(-1.0, i);
      float power = 2*i+1;
      pi += 16*sign * pow(x1, power)/power - 4*sign * pow(x2, power)/power;
      }
    pi;
)";
  auto tokens = tokenize(script);
  auto prog = make_program(tokens);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  optimize(prog);
  visitor<Program, pretty_print_visitor>::visit(prog, &ppv);
  VM::vmcode code;
  compile(code, prog);
  peephole_optimization(code);
  code.stream(std::cout);
  uint64_t size;
  uint8_t* f = (uint8_t*)VM::vm_bytecode(size, code);
  VM::registers regs;
  VM::run_bytecode(f, size, regs);
  std::cout << "Byte size: " << size << "\n";
  VM::free_bytecode(f, size);
  }

COMPILER_END

void run_all_optimize_tests()
  {
  using namespace COMPILER;
  //test_visitor_print();
  //test_visitor_print2();
  //test_optimize_harmonic();
  //test_optimize_fibonacci();
  //test_optimize_hamming();
  //test_optimize_qsort();
  //test_optimize_qsort_double();
  //test_optimize_add_zero();
  //test_optimize_mul_zero();
  //test_optimize_div_one();
  //test_optimize_constant_variables();
  //test_optimize_unused_variables();
  //test_optimize_number_transition();
  //test_optimize_super_operators();
  //test_optimize_digitspi();
  }