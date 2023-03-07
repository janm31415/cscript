#ifndef CSCRIPT_VISITOR_H
#define CSCRIPT_VISITOR_H

#include "cscript.h"
#include "parser.h"
#include "vector.h"

enum cscript_visitor_entry_type
  {
  CSCRIPT_VISITOR_STATEMENT_PRE,
  CSCRIPT_VISITOR_STATEMENT_POST,
  CSCRIPT_VISITOR_STATEMENTS_PRE,
  CSCRIPT_VISITOR_STATEMENTS_POST,
  CSCRIPT_VISITOR_SCOPED_STATEMENTS_PRE,
  CSCRIPT_VISITOR_SCOPED_STATEMENTS_POST,
  CSCRIPT_VISITOR_EXPRESSION_PRE,
  CSCRIPT_VISITOR_EXPRESSION_POST,
  CSCRIPT_VISITOR_ASSIGNMENT_PRE,
  CSCRIPT_VISITOR_ASSIGNMENT_POST,
  CSCRIPT_VISITOR_LVALUEOP_PRE,
  CSCRIPT_VISITOR_LVALUEOP_POST,
  CSCRIPT_VISITOR_NUMBER,
  CSCRIPT_VISITOR_TERM_PRE,
  CSCRIPT_VISITOR_TERM_POST,
  CSCRIPT_VISITOR_RELOP_PRE,
  CSCRIPT_VISITOR_RELOP_POST,
  CSCRIPT_VISITOR_VAR_PRE,
  CSCRIPT_VISITOR_VAR_POST,
  CSCRIPT_VISITOR_FIXNUM_PRE,
  CSCRIPT_VISITOR_FIXNUM_POST,
  CSCRIPT_VISITOR_FLONUM_PRE,
  CSCRIPT_VISITOR_FLONUM_POST,
  CSCRIPT_VISITOR_FACTOR_PRE,
  CSCRIPT_VISITOR_FACTOR_POST,
  CSCRIPT_VISITOR_FOR_PRE,
  CSCRIPT_VISITOR_FOR_POST,
  CSCRIPT_VISITOR_IF_PRE,
  CSCRIPT_VISITOR_IF_POST,
  CSCRIPT_VISITOR_FUNCTION_PRE,
  CSCRIPT_VISITOR_FUNCTION_POST,
  CSCRIPT_VISITOR_EXPRESSIONLIST_PRE,
  CSCRIPT_VISITOR_EXPRESSIONLIST_POST,
  CSCRIPT_VISITOR_PARAMETER,
  CSCRIPT_VISITOR_NOP
  };


typedef struct cscript_visitor_entry
  {
  void* entry;  
  enum cscript_visitor_entry_type type;
  } cscript_visitor_entry;

typedef struct cscript_visitor cscript_visitor;

struct cscript_visitor
  {
  int (*previsit_statement)(cscript_context*, cscript_visitor*, cscript_statement*);
  void (*postvisit_statement)(cscript_context*, cscript_visitor*, cscript_statement*);
  int (*previsit_statements)(cscript_context*, cscript_visitor*, cscript_comma_separated_statements*);
  void (*postvisit_statements)(cscript_context*, cscript_visitor*, cscript_comma_separated_statements*);
  int (*previsit_scoped_statements)(cscript_context*, cscript_visitor*, cscript_scoped_statements*);
  void (*postvisit_scoped_statements)(cscript_context*, cscript_visitor*, cscript_scoped_statements*);
  int (*previsit_expression)(cscript_context*, cscript_visitor*, cscript_parsed_expression*);
  void (*postvisit_expression)(cscript_context*, cscript_visitor*, cscript_parsed_expression*);
  int (*previsit_assignment)(cscript_context*, cscript_visitor*, cscript_parsed_assignment*);
  void (*postvisit_assignment)(cscript_context*, cscript_visitor*, cscript_parsed_assignment*);
  int (*previsit_lvalueop)(cscript_context*, cscript_visitor*, cscript_parsed_lvalue_operator*);
  void (*postvisit_lvalueop)(cscript_context*, cscript_visitor*, cscript_parsed_lvalue_operator*);
  void (*visit_number)(cscript_context*, cscript_visitor*, cscript_parsed_number*);
  int (*previsit_term)(cscript_context*, cscript_visitor*, cscript_parsed_term*);
  void (*postvisit_term)(cscript_context*, cscript_visitor*, cscript_parsed_term*);
  int (*previsit_relop)(cscript_context*, cscript_visitor*, cscript_parsed_relop*);
  void (*postvisit_relop)(cscript_context*, cscript_visitor*, cscript_parsed_relop*); 
  int (*previsit_var)(cscript_context*, cscript_visitor*, cscript_parsed_variable*);
  void (*postvisit_var)(cscript_context*, cscript_visitor*, cscript_parsed_variable*);
  int (*previsit_fixnum)(cscript_context*, cscript_visitor*, cscript_parsed_fixnum*);
  void (*postvisit_fixnum)(cscript_context*, cscript_visitor*, cscript_parsed_fixnum*);
  int (*previsit_flonum)(cscript_context*, cscript_visitor*, cscript_parsed_flonum*);
  void (*postvisit_flonum)(cscript_context*, cscript_visitor*, cscript_parsed_flonum*);
  int (*previsit_factor)(cscript_context*, cscript_visitor*, cscript_parsed_factor*);
  void (*postvisit_factor)(cscript_context*, cscript_visitor*, cscript_parsed_factor*);
  int (*previsit_for)(cscript_context*, cscript_visitor*, cscript_parsed_for*);
  void (*postvisit_for)(cscript_context*, cscript_visitor*, cscript_parsed_for*);
  int (*previsit_if)(cscript_context*, cscript_visitor*, cscript_parsed_if*);
  void (*postvisit_if)(cscript_context*, cscript_visitor*, cscript_parsed_if*);
  int (*previsit_function)(cscript_context*, cscript_visitor*, cscript_parsed_function*);
  void (*postvisit_function)(cscript_context*, cscript_visitor*, cscript_parsed_function*);
  int (*previsit_expression_list)(cscript_context*, cscript_visitor*, cscript_parsed_expression_list*);
  void (*postvisit_expression_list)(cscript_context*, cscript_visitor*, cscript_parsed_expression_list*);
  void (*visit_parameter)(cscript_context*, cscript_visitor*, cscript_parameter*);
  void (*visit_nop)(cscript_context*, cscript_visitor*, cscript_parsed_nop*);

  void (*destroy)(cscript_context*, cscript_visitor*);
  void* impl; // Implementation, for example cscript_dump_visitor

  cscript_vector v;
  };

cscript_visitor* cscript_visitor_new(cscript_context* ctxt, void* impl);

void cscript_visit_factor(cscript_context* ctxt, cscript_visitor* vis, cscript_parsed_factor* f);
void cscript_visit_term(cscript_context* ctxt, cscript_visitor* vis, cscript_parsed_term* t);
void cscript_visit_relop(cscript_context* ctxt, cscript_visitor* vis, cscript_parsed_relop* r);
void cscript_visit_expression(cscript_context* ctxt, cscript_visitor* vis, cscript_parsed_expression* e);
void cscript_visit_statement(cscript_context* ctxt, cscript_visitor* vis, cscript_statement* stmt);
void cscript_visit_program(cscript_context* ctxt, cscript_visitor* vis, cscript_program* p);

#endif //CSCRIPT_VISITOR_H