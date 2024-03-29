#ifndef CSCRIPT_CONTEXT_H
#define CSCRIPT_CONTEXT_H

#include "cscript.h"
#include "object.h"
#include "map.h"
#include "vector.h"

#include <setjmp.h>

typedef struct cscript_longjmp
  {
  cscript_alignment dummy;
  struct cscript_longjmp* previous;
  jmp_buf jmp;
  volatile int status; // error code  
  } cscript_longjmp;

// global state
typedef struct cscript_global_context
  {
  cscript_map_node dummy_node[1]; //common node array for all empty tables
  cscript_context* main_context;
  cscript_map* primitives_map;
  } cscript_global_context; 

// per thread state
struct cscript_context 
  {
  cscript_global_context* global; 
  cscript_vector stack;
  cscript_vector globals;
  int number_of_syntax_errors;
  int number_of_compile_errors;
  int number_of_runtime_errors;
  cscript_vector syntax_error_reports;
  cscript_vector compile_error_reports;
  cscript_vector runtime_error_reports;
  struct cscript_longjmp* error_jmp;  // current error recover point
  cscript_vector filenames_list;
  cscript_vector environment; // linked chain of environment maps
  cscript_vector externals;
  cscript_map* externals_map;
  };

#endif //CSCRIPT_CONTEXT_H