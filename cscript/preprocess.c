#include "preprocess.h"
#include "context.h"
#include "constfold.h"
#include "constprop.h"

void cscript_preprocess(cscript_context* ctxt, cscript_program* prog)
  {
  cscript_constant_propagation(ctxt, prog);
  cscript_constant_folding(ctxt, prog);
  }