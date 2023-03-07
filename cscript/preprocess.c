#include "preprocess.h"
#include "context.h"
#include "constfold.h"
#include "constprop.h"
#include "remdeadvar.h"
#include "alpha.h"

void cscript_preprocess(cscript_context* ctxt, cscript_program* prog)
  {
  cscript_alpha_conversion(ctxt, prog);
  for (int i = 0; i < 2; ++i)
    {
    cscript_constant_propagation(ctxt, prog);
    cscript_constant_folding(ctxt, prog);
    }
  cscript_remove_dead_variables(ctxt, prog);
  }