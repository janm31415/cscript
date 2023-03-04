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
  cscript_vector dims; //  vector of type cscript_parsed_expression
  int dereference;
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

typedef struct cscript_parsed_assignment
  {
  cscript_string name;
  cscript_string op;
  cscript_parsed_expression expr;
  cscript_vector dims; //  vector of type cscript_parsed_expression
  int derefence;
  int line_nr, column_nr;
  cscript_string filename;
  } cscript_parsed_assignment;

typedef struct cscript_parsed_lvalue_operator
  {
  cscript_string name;
  cscript_parsed_variable lvalue;
  int line_nr, column_nr;
  cscript_string filename;
  } cscript_parsed_lvalue_operator;

typedef struct cscript_parsed_function
  {
  cscript_string name;
  cscript_vector args;
  int line_nr, column_nr;
  cscript_string filename;
  } cscript_parsed_function;

typedef struct cscript_parsed_expression_list
  {
  cscript_vector expressions;
  int line_nr, column_nr;
  cscript_string filename;
  } cscript_parsed_expression_list;

#define cscript_factor_type_number 0
#define cscript_factor_type_expression 1
#define cscript_factor_type_variable 2
#define cscript_factor_type_lvalue_operator 3
#define cscript_factor_type_function 4
#define cscript_factor_type_expression_list 5

typedef union
  {
  cscript_parsed_number number;
  cscript_parsed_expression expr;
  cscript_parsed_variable var;
  cscript_parsed_lvalue_operator lvop;
  cscript_parsed_function fun;
  cscript_parsed_expression_list exprlist;
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

typedef struct cscript_comma_separated_statements
  {
  cscript_vector statements;
  } cscript_comma_separated_statements;

typedef struct cscript_parsed_for
  {
  cscript_vector init_cond_inc;
  cscript_vector statements;
  int line_nr, column_nr;
  cscript_string filename;
  } cscript_parsed_for;

typedef struct cscript_parsed_if
  {
  cscript_vector condition;
  cscript_vector body;
  cscript_vector alternative;
  int line_nr, column_nr;
  cscript_string filename;
  } cscript_parsed_if;

#define cscript_statement_type_expression 0
#define cscript_statement_type_fixnum 1
#define cscript_statement_type_flonum 2
#define cscript_statement_type_nop 3
#define cscript_statement_type_if 4
#define cscript_statement_type_for 5
#define cscript_statement_type_comma_separated 6
#define cscript_statement_type_assignment 7

typedef union
  {
  cscript_parsed_expression expr;
  cscript_parsed_fixnum fixnum;
  cscript_parsed_flonum flonum;
  cscript_parsed_nop nop;
  cscript_comma_separated_statements stmts;
  cscript_parsed_assignment assignment;
  cscript_parsed_for forloop;
  cscript_parsed_if iftest;
  } cscript_parsed_statement;

typedef struct cscript_statement
  {
  cscript_parsed_statement statement;
  int type;
  } cscript_statement;

#define cscript_parameter_type_fixnum 0
#define cscript_parameter_type_flonum 1
#define cscript_parameter_type_fixnum_pointer 2
#define cscript_parameter_type_flonum_pointer 3

typedef struct cscript_parameter
  {
  cscript_string name;
  int type;
  int line_nr, column_nr;
  cscript_string filename;
  } cscript_parameter;

typedef struct cscript_program
  {
  cscript_vector parameters;
  cscript_vector statements;
  } cscript_program;

cscript_statement cscript_make_statement(cscript_context* ctxt, token** token_it, token** token_it_end);
CSCRIPT_API cscript_program make_program(cscript_context* ctxt, cscript_vector* tokens);
CSCRIPT_API void cscript_program_destroy(cscript_context* ctxt, cscript_program* p);
CSCRIPT_API void cscript_statement_destroy(cscript_context* ctxt, cscript_statement* e);

#endif //CSCRIPT_PARSER_H