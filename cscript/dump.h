#ifndef CSCRIPT_DUMP_H
#define CSCRIPT_DUMP_H

#include "cscript.h"
#include "visitor.h"
#include "string.h"

typedef struct cscript_dump_visitor
  {
  cscript_visitor* visitor;
  cscript_string s;
  } cscript_dump_visitor;

cscript_dump_visitor* cscript_dump_visitor_new(cscript_context* ctxt);
void cscript_dump_visitor_free(cscript_context* ctxt, cscript_dump_visitor* v);

CSCRIPT_API cscript_string cscript_dump(cscript_context* ctxt, cscript_program* prog);
CSCRIPT_API cscript_string cscript_dump_statement(cscript_context* ctxt, cscript_statement* e);


#endif //CSCRIPT_DUMP_H