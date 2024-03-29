#ifndef CSCRIPT_COMPILER_H
#define CSCRIPT_COMPILER_H

#include "func.h"
#include "parser.h"
#include "vector.h"

CSCRIPT_API cscript_function* cscript_compile_program(cscript_context* ctxt, cscript_program* prog);

#endif //CSCRIPT_COMPILER_H