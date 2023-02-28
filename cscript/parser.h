#ifndef CSCRIPT_PARSER_H
#define CSCRIPT_PARSER_H

#include "cscript.h"
#include "string.h"
#include "vector.h"
#include "map.h"
#include "token.h"

#define cscript_number_type_fixnum 0
#define cscript_number_type_flonum 1

#define cscript_op_mul 0
#define cscript_op_div 1
#define cscript_op_percent 2
#define cscript_op_plus 3
#define cscript_op_minus 4
#define cscript_op_less 5
#define cscript_op_leq 6
#define cscript_op_greater 7
#define cscript_op_geq 8
#define cscript_op_equal 9
#define cscript_op_not_equal 10

typedef union
  {
  cscript_fixnum fx;
  cscript_flonum fl;
  } cscript_number;

typedef struct cscript_parsed_number
  {
  int type;
  cscript_number number;
  int line_nr, column_nr;
  cscript_string filename;
  } cscript_parsed_number;

typedef struct cscript_parsed_term
  {
  cscript_vector operands; // vector of type cscript_parsed_factor
  cscript_vector fops;
  int line_nr, column_nr;
  cscript_string filename;
  } cscript_parsed_term;

typedef struct cscript_parsed_relop
  {
  cscript_vector operands; // vector of type cscript_parsed_term
  cscript_vector fops;
  int line_nr, column_nr;
  cscript_string filename;
  } cscript_parsed_relop;

typedef struct cscript_parsed_expression
  {
  cscript_vector operands; // vector of type cscript_parsed_relop
  cscript_vector fops;
  int line_nr, column_nr;
  cscript_string filename;
  } cscript_parsed_expression;

typedef struct cscript_parsed_variable
  {
  cscript_string name;
  int line_nr, column_nr;
  cscript_string filename;
  } cscript_parsed_variable;

typedef struct cscript_parsed_fixnum
  {
  cscript_string name;
  cscript_parsed_expression expr;
  cscript_vector dims; //  vector of type cscript_parsed_expression
  int line_nr, column_nr;
  cscript_string filename;
  } cscript_parsed_fixnum;

typedef struct cscript_parsed_flonum
  {
  cscript_string name;
  cscript_parsed_expression expr;
  cscript_vector dims; //  vector of type cscript_parsed_expression
  int line_nr, column_nr;
  cscript_string filename;
  } cscript_parsed_flonum;

#define cscript_factor_type_number 0
#define cscript_factor_type_expression 1
#define cscript_factor_type_variable 2

typedef union
  {
  cscript_parsed_number number;
  cscript_parsed_expression expr;
  cscript_parsed_variable var;
  } cscript_factor;

typedef struct cscript_parsed_factor
  {
  int type;
  cscript_factor factor;
  char sign;
  } cscript_parsed_factor;

typedef struct cscript_parsed_nop
  {
  int line_nr, column_nr;
  cscript_string filename;
  } cscript_parsed_nop;

#define cscript_statement_type_expression 0
#define cscript_statement_type_fixnum 1
#define cscript_statement_type_flonum 2
#define cscript_statement_type_nop 3
#define cscript_statement_type_if 4
#define cscript_statement_type_for 5

typedef union
  {
  cscript_parsed_expression expr;
  cscript_parsed_fixnum fixnum;
  cscript_parsed_flonum flonum;
  cscript_parsed_nop nop;
  } cscript_parsed_statement;

typedef struct cscript_statement
  {
  cscript_parsed_statement statement;
  int type;
  } cscript_statement;

typedef struct cscript_program
  {
  cscript_vector statements;
  } cscript_program;

cscript_statement cscript_make_statement(cscript_context* ctxt, token** token_it, token** token_it_end);
CSCRIPT_API cscript_program make_program(cscript_context* ctxt, cscript_vector* tokens);
CSCRIPT_API void cscript_program_destroy(cscript_context* ctxt, cscript_program* p);
CSCRIPT_API void cscript_statement_destroy(cscript_context* ctxt, cscript_statement* e);

#endif //CSCRIPT_PARSER_H