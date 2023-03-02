#ifndef CSCRIPT_COMPILER_H
#define CSCRIPT_COMPILER_H

#include "func.h"
#include "parser.h"
#include "vector.h"

CSCRIPT_API cscript_function* cscript_compile_statement(cscript_context* ctxt, cscript_statement* stmt);
CSCRIPT_API cscript_function* cscript_compile_program(cscript_context* ctxt, cscript_program* prog);
CSCRIPT_API void cscript_compiled_program_destroy(cscript_context* ctxt, cscript_vector* compiled_program);

#endif //CSCRIPT_COMPILER_H