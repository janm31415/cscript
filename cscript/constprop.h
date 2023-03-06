#ifndef CSCRIPT_CONSTPROP_H
#define CSCRIPT_CONSTPROP_H

#include "cscript.h"
#include "parser.h"

void cscript_constant_propagation(cscript_context* ctxt, cscript_program* program);

#endif //CSCRIPT_CONSTPROP_H