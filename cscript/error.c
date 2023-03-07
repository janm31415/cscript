#include "error.h"
#include "limits.h"
#include "context.h"
#include "syscalls.h"

#include <stdlib.h>
#include <setjmp.h>
#include <stdio.h>

void cscript_throw(cscript_context* ctxt, int errorcode)
  {
  UNUSED(ctxt);
  UNUSED(errorcode);
  if (ctxt->error_jmp)
    {
    ctxt->error_jmp->status = errorcode;
    longjmp(ctxt->error_jmp->jmp, 1);
    }
  exit(errorcode);
  }

static void append_error_code(cscript_context* ctxt, cscript_string* message, int errorcode)
  {
  switch (errorcode)
    {
    case CSCRIPT_ERROR_MEMORY: cscript_string_append_cstr(ctxt, message, "out of memory"); break;
    case CSCRIPT_ERROR_NO_TOKENS: cscript_string_append_cstr(ctxt, message, "no tokens"); break;
    case CSCRIPT_ERROR_NOT_IMPLEMENTED: cscript_string_append_cstr(ctxt, message, "not implemented"); break;
    case CSCRIPT_ERROR_BAD_SYNTAX: cscript_string_append_cstr(ctxt, message, "bad syntax"); break;
    case CSCRIPT_ERROR_EXPECTED_KEYWORD: cscript_string_append_cstr(ctxt, message, "expected keyword"); break;
    case CSCRIPT_ERROR_RUNERROR: cscript_string_append_cstr(ctxt, message, "runtime error"); break;
    case CSCRIPT_ERROR_INVALID_NUMBER_OF_ARGUMENTS: cscript_string_append_cstr(ctxt, message, "invalid number of arguments"); break;
    case CSCRIPT_ERROR_INVALID_ARGUMENT: cscript_string_append_cstr(ctxt, message, "invalid argument"); break;
    case CSCRIPT_ERROR_VARIABLE_UNKNOWN: cscript_string_append_cstr(ctxt, message, "variable unknown"); break;
    case CSCRIPT_ERROR_EXTERNAL_UNKNOWN: cscript_string_append_cstr(ctxt, message, "external unknown"); break;
    default: break;
    }
  }

void cscript_syntax_error(cscript_context* ctxt, int errorcode, int line_nr, int column_nr, cscript_string* filename, cscript_string* msg)
  {
  cscript_string message;
  cscript_string_init(ctxt, &message, "syntax error");
  if (filename && filename->string_length > 0)
    {
    cscript_string_append_cstr(ctxt, &message, " (");
    cscript_string_append(ctxt, &message, filename);
    cscript_string_push_back(ctxt, &message, ')');
    }
  if (line_nr >= 0 && column_nr >= 0)
    {
    char number[256];
    cscript_string_append_cstr(ctxt, &message, " (");
    cscript_int_to_char(number, line_nr);
    cscript_string_append_cstr(ctxt, &message, number);
    cscript_string_push_back(ctxt, &message, ',');
    cscript_int_to_char(number, column_nr);
    cscript_string_append_cstr(ctxt, &message, number);
    cscript_string_push_back(ctxt, &message, ')');
    }
  cscript_string_append_cstr(ctxt, &message, ": ");
  append_error_code(ctxt, &message, errorcode);  
  if (msg->string_length > 0)
    {
    cscript_string_append_cstr(ctxt, &message, ": ");
    cscript_string_append(ctxt, &message, msg);
    }
  cscript_string_destroy(ctxt, msg);
  ++ctxt->number_of_syntax_errors;
  cscript_error_report report;
  report.column_number = column_nr;
  report.line_number = line_nr;
  report.errorcode = errorcode;
  report.message = message;
  cscript_vector_push_back(ctxt, &ctxt->syntax_error_reports, report, cscript_error_report);
  }

void cscript_syntax_error_cstr(cscript_context* ctxt, int errorcode, int line_nr, int column_nr, cscript_string* filename, const char* msg)
  {
  cscript_string s;
  cscript_string_init(ctxt, &s, msg);
  cscript_syntax_error(ctxt, errorcode, line_nr, column_nr, filename, &s);
  }

void cscript_compile_error(cscript_context* ctxt, int errorcode, int line_nr, int column_nr, cscript_string* filename, cscript_string* msg)
  {
  cscript_string message;
  cscript_string_init(ctxt, &message, "compile error");
  if (filename && filename->string_length > 0)
    {
    cscript_string_append_cstr(ctxt, &message, " (");
    cscript_string_append(ctxt, &message, filename);
    cscript_string_push_back(ctxt, &message, ')');
    }
  if (line_nr >= 0 && column_nr >= 0)
    {
    char number[256];
    cscript_string_append_cstr(ctxt, &message, " (");
    cscript_int_to_char(number, line_nr);
    cscript_string_append_cstr(ctxt, &message, number);
    cscript_string_push_back(ctxt, &message, ',');
    cscript_int_to_char(number, column_nr);
    cscript_string_append_cstr(ctxt, &message, number);
    cscript_string_push_back(ctxt, &message, ')');
    }
  cscript_string_append_cstr(ctxt, &message, ": ");
  append_error_code(ctxt, &message, errorcode);
  if (msg->string_length > 0)
    {
    cscript_string_append_cstr(ctxt, &message, ": ");
    cscript_string_append(ctxt, &message, msg);
    }
  cscript_string_destroy(ctxt, msg);
  ++ctxt->number_of_compile_errors;
  cscript_error_report report;
  report.column_number = column_nr;
  report.line_number = line_nr;
  report.errorcode = errorcode;
  report.message = message;
  cscript_vector_push_back(ctxt, &ctxt->compile_error_reports, report, cscript_error_report);
  }

void cscript_compile_error_cstr(cscript_context* ctxt, int errorcode, int line_nr, int column_nr, cscript_string* filename, const char* msg)
  {
  cscript_string s;
  cscript_string_init(ctxt, &s, msg);
  cscript_compile_error(ctxt, errorcode, line_nr, column_nr, filename, &s);
  }

void cscript_runtime_error(cscript_context* ctxt, int errorcode, int line_nr, int column_nr, cscript_string* msg)
  {
  cscript_string message;
  cscript_string_init(ctxt, &message, "runtime error");
  if (line_nr >= 0 && column_nr >= 0)
    {
    char number[256];
    cscript_string_append_cstr(ctxt, &message, " (");
    cscript_int_to_char(number, line_nr);
    cscript_string_append_cstr(ctxt, &message, number);
    cscript_string_push_back(ctxt, &message, ',');
    cscript_int_to_char(number, column_nr);
    cscript_string_append_cstr(ctxt, &message, number);
    cscript_string_push_back(ctxt, &message, ')');
    }
  cscript_string_append_cstr(ctxt, &message, ": ");
  append_error_code(ctxt, &message, errorcode);
  if (msg->string_length > 0)
    {
    cscript_string_append_cstr(ctxt, &message, ": ");
    cscript_string_append(ctxt, &message, msg);
    }
  cscript_string_destroy(ctxt, msg);
  ++ctxt->number_of_runtime_errors;
  cscript_error_report report;
  report.column_number = column_nr;
  report.line_number = line_nr;
  report.errorcode = errorcode;
  report.message = message;
  cscript_vector_push_back(ctxt, &ctxt->runtime_error_reports, report, cscript_error_report);
  }

void cscript_runtime_error_cstr(cscript_context* ctxt, int errorcode, int line_nr, int column_nr, const char* msg)
  {
  cscript_string s;
  cscript_string_init(ctxt, &s, msg);
  cscript_runtime_error(ctxt, errorcode, line_nr, column_nr, &s);
  }

void cscript_syntax_errors_clear(cscript_context* ctxt)
  {
  cscript_error_report* it = cscript_vector_begin(&ctxt->syntax_error_reports, cscript_error_report);
  cscript_error_report* it_end = cscript_vector_end(&ctxt->syntax_error_reports, cscript_error_report);
  for (; it != it_end; ++it)
    {
    cscript_string_destroy(ctxt, &it->message);
    }
  ctxt->syntax_error_reports.vector_size = 0;
  ctxt->number_of_syntax_errors = 0;
  }

void cscript_compile_errors_clear(cscript_context* ctxt)
  {
  cscript_error_report* it = cscript_vector_begin(&ctxt->compile_error_reports, cscript_error_report);
  cscript_error_report* it_end = cscript_vector_end(&ctxt->compile_error_reports, cscript_error_report);
  for (; it != it_end; ++it)
    {
    cscript_string_destroy(ctxt, &it->message);
    }
  ctxt->compile_error_reports.vector_size = 0;
  ctxt->number_of_compile_errors = 0;
  }

void cscript_runtime_errors_clear(cscript_context* ctxt)
  {
  cscript_error_report* it = cscript_vector_begin(&ctxt->runtime_error_reports, cscript_error_report);
  cscript_error_report* it_end = cscript_vector_end(&ctxt->runtime_error_reports, cscript_error_report);
  for (; it != it_end; ++it)
    {
    cscript_string_destroy(ctxt, &it->message);
    }
  ctxt->runtime_error_reports.vector_size = 0;
  ctxt->number_of_runtime_errors = 0;
  }

void cscript_get_error_string(cscript_context* ctxt, cscript_string* s)
  {
  if (ctxt->number_of_compile_errors > 0)
    {
    cscript_error_report* it = cscript_vector_begin(&ctxt->compile_error_reports, cscript_error_report);
    cscript_error_report* it_end = cscript_vector_end(&ctxt->compile_error_reports, cscript_error_report);
    for (; it != it_end; ++it)
      {
      cscript_string_append(ctxt, s, &it->message);
      cscript_string_push_back(ctxt, s, '\n');
      }
    }
  if (ctxt->number_of_syntax_errors > 0)
    {
    cscript_error_report* it = cscript_vector_begin(&ctxt->syntax_error_reports, cscript_error_report);
    cscript_error_report* it_end = cscript_vector_end(&ctxt->syntax_error_reports, cscript_error_report);
    for (; it != it_end; ++it)
      {
      cscript_string_append(ctxt, s, &it->message);
      cscript_string_push_back(ctxt, s, '\n');
      }
    }
  if (ctxt->number_of_runtime_errors > 0)
    {
    cscript_error_report* it = cscript_vector_begin(&ctxt->runtime_error_reports, cscript_error_report);
    cscript_error_report* it_end = cscript_vector_end(&ctxt->runtime_error_reports, cscript_error_report);
    for (; it != it_end; ++it)
      {
      cscript_string_append(ctxt, s, &it->message);
      cscript_string_push_back(ctxt, s, '\n');
      }
    }
  }

void cscript_print_any_error(cscript_context* ctxt)
  {
  cscript_string s;
  cscript_string_init(ctxt, &s, "");
  cscript_get_error_string(ctxt, &s);
  if (s.string_length > 0)
    printf("%s", s.string_ptr);
  cscript_string_destroy(ctxt, &s);
  }

int cscript_context_is_error_free(cscript_context* ctxt)
  {
  if (ctxt->number_of_compile_errors > 0)
    return 0;
  if (ctxt->number_of_syntax_errors > 0)
    return 0;
  if (ctxt->number_of_runtime_errors > 0)
    return 0;
  return 1;
  }