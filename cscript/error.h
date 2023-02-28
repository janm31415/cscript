#ifndef CSCRIPT_ERROR_H
#define CSCRIPT_ERROR_H

#include "cscript.h"
#include "string.h"

#define CSCRIPT_ERROR_MEMORY 1
#define CSCRIPT_ERROR_NO_TOKENS 2
#define CSCRIPT_ERROR_NOT_IMPLEMENTED 3
#define CSCRIPT_ERROR_BAD_SYNTAX 4
#define CSCRIPT_ERROR_EXPECTED_KEYWORD 5
#define CSCRIPT_ERROR_RUNERROR 6
#define CSCRIPT_ERROR_INVALID_NUMBER_OF_ARGUMENTS 7
#define CSCRIPT_ERROR_INVALID_ARGUMENT 8
#define CSCRIPT_ERROR_VARIABLE_UNKNOWN 9
#define CSCRIPT_ERROR_EXTERNAL_UNKNOWN 10

void cscript_throw(cscript_context* ctxt, int errorcode);

void cscript_syntax_error(cscript_context* ctxt, int errorcode, int line_nr, int column_nr, cscript_string* filename, cscript_string* msg);
void cscript_syntax_error_cstr(cscript_context* ctxt, int errorcode, int line_nr, int column_nr, cscript_string* filename, const char* msg);
void cscript_compile_error(cscript_context* ctxt, int errorcode, int line_nr, int column_nr, cscript_string* filename, cscript_string* msg);
void cscript_compile_error_cstr(cscript_context* ctxt, int errorcode, int line_nr, int column_nr, cscript_string* filename, const char* msg);
void cscript_runtime_error(cscript_context* ctxt, int errorcode, int line_nr, int column_nr, cscript_string* msg);
void cscript_runtime_error_cstr(cscript_context* ctxt, int errorcode, int line_nr, int column_nr, const char* msg);

typedef struct cscript_error_report
  {
  int errorcode;
  int line_number;
  int column_number;
  cscript_string message;
  } cscript_error_report;

void cscript_syntax_errors_clear(cscript_context* ctxt);
void cscript_compile_errors_clear(cscript_context* ctxt);
void cscript_runtime_errors_clear(cscript_context* ctxt);
int cscript_context_is_error_free(cscript_context* ctxt);

void cscript_print_any_error(cscript_context* ctxt);
void cscript_get_error_string(cscript_context* ctxt, cscript_string* s);

#endif //CSCRIPT_ERROR_H