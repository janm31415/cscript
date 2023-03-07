#ifndef CSCRIPT_TOKEN_H
#define CSCRIPT_TOKEN_H

#include "cscript.h"
#include "string.h"
#include "stream.h"
#include "vector.h"

CSCRIPT_API cscript_flonum cscript_to_flonum(const char* value);
CSCRIPT_API cscript_fixnum cscript_to_fixnum(const char* value);
CSCRIPT_API int cscript_is_number(int* is_real, int* is_scientific, const char* value);

enum token_type
  {
  CSCRIPT_T_BAD,  
  CSCRIPT_T_LEFT_ROUND_BRACKET,
  CSCRIPT_T_RIGHT_ROUND_BRACKET,
  CSCRIPT_T_LEFT_SQUARE_BRACKET,
  CSCRIPT_T_RIGHT_SQUARE_BRACKET,
  CSCRIPT_T_LEFT_CURLY_BRACE,
  CSCRIPT_T_RIGHT_CURLY_BRACE,
  CSCRIPT_T_RELATIVE_LESS,
  CSCRIPT_T_RELATIVE_LEQ,
  CSCRIPT_T_RELATIVE_GREATER,
  CSCRIPT_T_RELATIVE_GEQ,
  CSCRIPT_T_RELATIVE_EQUAL,
  CSCRIPT_T_RELATIVE_NOTEQUAL,
  CSCRIPT_T_FIXNUM,
  CSCRIPT_T_FLONUM,
  CSCRIPT_T_ID,
  CSCRIPT_T_ASSIGNMENT,
  CSCRIPT_T_ASSIGNMENT_PLUS,
  CSCRIPT_T_ASSIGNMENT_MINUS,
  CSCRIPT_T_ASSIGNMENT_MUL,
  CSCRIPT_T_ASSIGNMENT_DIV,
  CSCRIPT_T_INCREMENT,
  CSCRIPT_T_DECREMENT,
  CSCRIPT_T_PLUS,
  CSCRIPT_T_MINUS,
  CSCRIPT_T_MUL,
  CSCRIPT_T_DIV,
  CSCRIPT_T_NOT,
  CSCRIPT_T_SEMICOLON,
  CSCRIPT_T_COMMA,
  CSCRIPT_T_AMPERSAND,
  CSCRIPT_T_PERCENT
  };


typedef struct token 
  {
  cscript_string value;
  int type;
  int line_nr;
  int column_nr;
  } token;

token cscript_make_token(cscript_context* ctxt, int type, int line_nr, int column_nr, cscript_string* value);
token cscript_make_token_cstr(cscript_context* ctxt, int type, int line_nr, int column_nr, const char* value);

typedef struct cscript_read_token_state
  {
  int line_nr;
  int column_nr;
  } cscript_read_token_state;

int cscript_read_token(token* tok, cscript_context* ctxt, cscript_string* buff, cscript_stream* str, cscript_read_token_state* state);

typedef int (*cscript_get_char_fun)(char*, void*, int);
typedef void (*cscript_next_char_fun)(void*);
typedef cscript_fixnum (*cscript_get_position_fun)(void*);
int cscript_read_token_polymorph(token* tok, cscript_context* ctxt, cscript_string* buff, void* str, cscript_read_token_state* state, cscript_get_char_fun get_char, cscript_next_char_fun next_char, cscript_get_position_fun get_position);

void cscript_token_destroy(cscript_context* ctxt, token* tok);

/*
* return vector contains token types
*/
CSCRIPT_API cscript_vector tokenize(cscript_context* ctxt, cscript_stream* str);

CSCRIPT_API void destroy_tokens_vector(cscript_context* ctxt, cscript_vector* tokens);

CSCRIPT_API cscript_vector cscript_script2tokens(cscript_context* ctxt, const char* script);

#endif //CSCRIPT_TOKEN_H