#ifndef CSCRIPT_ALPHA_H
#define CSCRIPT_ALPHA_H

#include "cscript.h"
#include "parser.h"

cscript_string cscript_make_alpha_name(cscript_context* ctxt, cscript_string* original, cscript_memsize index);
cscript_string cscript_get_original_name_from_alpha(cscript_context* ctxt, cscript_string* alpha_name);

void cscript_alpha_conversion(cscript_context* ctxt, cscript_program* program);

#endif //CSCRIPT_ALPHA_H