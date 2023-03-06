#include "constfold.h"
#include "visitor.h"



typedef struct cscript_constant_folding_visitor
  {
  cscript_visitor* visitor;
  } cscript_constant_folding_visitor;

static cscript_constant_folding_visitor* cscript_constant_folding_visitor_new(cscript_context* ctxt)
  {
  cscript_constant_folding_visitor* v = cscript_new(ctxt, cscript_constant_folding_visitor);
  v->visitor = cscript_visitor_new(ctxt, v);
  return v;
  }

static void cscript_constant_folding_visitor_free(cscript_context* ctxt, cscript_constant_folding_visitor* v)
  {
  if (v)
    {
    v->visitor->destroy(ctxt, v->visitor);
    cscript_delete(ctxt, v);
    }
  }

void cscript_constant_folding(cscript_context* ctxt, cscript_program* program)
  {
  cscript_constant_folding_visitor* v = cscript_constant_folding_visitor_new(ctxt);
  cscript_visit_program(ctxt, v->visitor, program);
  cscript_constant_folding_visitor_free(ctxt, v);
  }