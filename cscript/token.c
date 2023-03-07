#include "token.h"
#include "limits.h"
#include "string.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

cscript_flonum cscript_to_flonum(const char* value)
  {
  return cast(cscript_flonum, atof(value));
  }

cscript_fixnum cscript_to_fixnum(const char* value)
  {
#ifdef _WIN32
  return cast(cscript_fixnum, _atoi64(value));
#else
  char* endptr;
  return cast(cscript_fixnum, strtoull(value, &endptr, 10));
#endif
  }

int cscript_is_number(int* is_real, int* is_scientific, const char* value)
  {
  if (value[0] == '\0')
    return 0;
  int i = 0;
  if (value[0] == 'e' || value[0] == 'E')
    return 0;
  if (value[0] == 'f' || value[0] == 'F')
    return 0;
  if (value[0] == '-' || value[0] == '+')
    {
    ++i;
    if (value[1] == '\0')
      return 0;
    }
  *is_real = 0;
  *is_scientific = 0;
  const char* s = value + i;
  while (*s != '\0')
    {
    if (isdigit((unsigned char)(*s)) == 0)
      {
      if ((*s == '.') && (*is_real == 0) && (*is_scientific == 0))
        *is_real = 1;
      else if ((*s == 'e' || *s == 'E') && (*is_scientific == 0))
        {
        *is_scientific = 1;
        *is_real = 1;
        if (*(s + 1) == '\0')
          return 0;
        if (*(s + 1) == '-' || *(s + 1) == '+')
          {
          ++s;
          }
        if (*(s + 1) == '\0')
          return 0;
        }
      else if (*is_real && (*s == 'f' || *s == 'F'))
        {
        ++s;
        if (*s == '\0')
          {
          return 1;
          }
        return 0;
        }
      else
        return 0;
      }
    ++s;
    }
  return 1;
  }

static int ignore_character(char ch)
  {
  return (ch == ' ' || ch == '\n' || ch == '\t' || ch == '\r');
  }

token cscript_make_token(cscript_context* ctxt, int type, int line_nr, int column_nr, cscript_string* value)
  {
  token t;
  t.type = type;
  t.line_nr = line_nr;
  t.column_nr = column_nr;
  cscript_string_init(ctxt, &t.value, value->string_ptr);
  return t;
  }

token cscript_make_token_cstr(cscript_context* ctxt, int type, int line_nr, int column_nr, const char* value)
  {
  token t;
  t.type = type;
  t.line_nr = line_nr;
  t.column_nr = column_nr;
  cscript_string_init(ctxt, &t.value, value);
  return t;
  }


static int treat_buffer_token(cscript_context* ctxt, cscript_string* buff, token* tok, int line_nr, int column_nr)
  {
  int result = 0;
  if (buff->string_length != 0 && *cscript_string_front(buff) != '\0')
    {
    int is_real;
    int is_scientific;
    if (cscript_is_number(&is_real, &is_scientific, buff->string_ptr))
      {
      if (is_real)
        {
        *tok = cscript_make_token(ctxt, CSCRIPT_T_FLONUM, line_nr, column_nr - (int)buff->string_length, buff);
        result = 1;
        }
      else
        {
        *tok = cscript_make_token(ctxt, CSCRIPT_T_FIXNUM, line_nr, column_nr - (int)buff->string_length, buff);
        result = 1;
        }
      }
    else
      {
      *tok = cscript_make_token(ctxt, CSCRIPT_T_ID, line_nr, column_nr - (int)buff->string_length, buff);
      result = 1;
      }
    }

  cscript_string_clear(buff);
  return result;
  }

int cscript_read_token_polymorph(token* tok, cscript_context* ctxt, cscript_string* buff, void* str, cscript_read_token_state* state, cscript_get_char_fun get_char, cscript_next_char_fun next_char, cscript_get_position_fun get_position)
  {
  tok->value.string_ptr = 0;
  tok->value.string_capacity = 0;
  tok->value.string_length = 0;
  tok->type = CSCRIPT_T_BAD;
  cscript_assert(buff->string_length == 0);

  char s;
  int valid_chars_remaining = get_char(&s, str, 0);
  while (valid_chars_remaining)
    {
    if (ignore_character(s))
      {
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      while (ignore_character(s) && valid_chars_remaining)
        {
        if (s == '\n')
          {
          ++state->line_nr;
          state->column_nr = 0;
          }
        next_char(str);
        valid_chars_remaining = get_char(&s, str, 0);
        ++state->column_nr;
        }
      if (!valid_chars_remaining)
        break;
      }

    const cscript_fixnum str_position = get_position(str);

    switch (s)
      {
      case '(':
      {
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      next_char(str);
      *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_LEFT_ROUND_BRACKET, state->line_nr, state->column_nr, "(");
      ++state->column_nr;
      return 1;
      }
      case ')':
      {
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      next_char(str);
      *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_RIGHT_ROUND_BRACKET, state->line_nr, state->column_nr, ")");
      ++state->column_nr;
      return 1;
      }
      case '[':
      {
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      next_char(str);
      *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_LEFT_SQUARE_BRACKET, state->line_nr, state->column_nr, "[");
      ++state->column_nr;
      return 1;
      }
      case ']':
      {
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      next_char(str);
      *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_RIGHT_SQUARE_BRACKET, state->line_nr, state->column_nr, "]");
      ++state->column_nr;
      return 1;
      }   
      case ';':
      {
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      next_char(str);
      *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_SEMICOLON, state->line_nr, state->column_nr, ";");
      ++state->column_nr;
      return 1;
      }      
      case ',':
      {
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      next_char(str);
      *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_COMMA, state->line_nr, state->column_nr, ",");
      ++state->column_nr;
      return 1;
      }
      case '{':
      {
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      next_char(str);
      *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_LEFT_CURLY_BRACE, state->line_nr, state->column_nr, "{");
      ++state->column_nr;
      return 1;
      }
      case '}':
      {
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      next_char(str);
      *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_RIGHT_CURLY_BRACE, state->line_nr, state->column_nr, "}");
      ++state->column_nr;
      return 1;
      }
      case '%':
      {
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      next_char(str);
      *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_PERCENT, state->line_nr, state->column_nr, "%");
      ++state->column_nr;
      return 1;
      }
      case '&':
      {
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      next_char(str);
      *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_AMPERSAND, state->line_nr, state->column_nr, "&");
      ++state->column_nr;
      return 1;
      }
      case '+':
      {
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      next_char(str);
      char t;
      int valid_peek = get_char(&t, str, 0); // peek
      if (valid_peek && t == '=')
        {
        *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_ASSIGNMENT_PLUS, state->line_nr, state->column_nr, "+=");
        next_char(str);
        state->column_nr += 2;
        return 1;
        }
      else if (valid_peek && t == '+')
        {
        *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_INCREMENT, state->line_nr, state->column_nr, "++");
        next_char(str);
        state->column_nr += 2;
        return 1;
        }
      *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_PLUS, state->line_nr, state->column_nr, "+");
      ++state->column_nr;
      return 1;
      }
      case '-':
      {
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      next_char(str);
      char t;
      int valid_peek = get_char(&t, str, 0); // peek
      if (valid_peek && t == '=')
        {
        *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_ASSIGNMENT_MINUS, state->line_nr, state->column_nr, "-=");
        next_char(str);
        state->column_nr += 2;
        return 1;
        }
      else if (valid_peek && t == '-')
        {
        *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_DECREMENT, state->line_nr, state->column_nr, "--");
        next_char(str);
        state->column_nr += 2;
        return 1;
        }
      *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_MINUS, state->line_nr, state->column_nr, "-");
      ++state->column_nr;
      return 1;
      }
      case '*':
      {
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      next_char(str);
      char t;
      int valid_peek = get_char(&t, str, 0); // peek
      if (valid_peek && t == '=')
        {
        *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_ASSIGNMENT_MUL, state->line_nr, state->column_nr, "*=");
        next_char(str);
        state->column_nr += 2;
        return 1;
        }
      *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_MUL, state->line_nr, state->column_nr, "*");
      ++state->column_nr;
      return 1;
      }
      case '<':
      {
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      next_char(str);
      char t;
      int valid_peek = get_char(&t, str, 0); // peek
      if (valid_peek && t == '=')
        {
        *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_RELATIVE_LEQ, state->line_nr, state->column_nr, "<=");
        next_char(str);
        state->column_nr += 2;
        return 1;
        }
      *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_RELATIVE_LESS, state->line_nr, state->column_nr, "<");
      ++state->column_nr;
      return 1;
      }
      case '>':
      {
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      next_char(str);
      char t;
      int valid_peek = get_char(&t, str, 0); // peek
      if (valid_peek && t == '=')
        {
        *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_RELATIVE_GEQ, state->line_nr, state->column_nr, ">=");
        next_char(str);
        state->column_nr += 2;
        return 1;
        }
      *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_RELATIVE_GREATER, state->line_nr, state->column_nr, ">");
      ++state->column_nr;
      return 1;
      }
      case '=':
      {
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      next_char(str);
      char t;
      int valid_peek = get_char(&t, str, 0); // peek
      if (valid_peek && t == '=')
        {
        *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_RELATIVE_EQUAL, state->line_nr, state->column_nr, "==");
        next_char(str);
        state->column_nr += 2;
        return 1;
        }
      *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_ASSIGNMENT, state->line_nr, state->column_nr, "=");
      ++state->column_nr;
      return 1;
      }
      case '!':
      {      
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      next_char(str);
      char t;
      int valid_peek = get_char(&t, str, 0); // peek
      if (valid_peek && t == '=')
        {
        *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_RELATIVE_NOTEQUAL, state->line_nr, state->column_nr, "!=");
        next_char(str);
        state->column_nr += 2;
        return 1;
        }
      *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_NOT, state->line_nr, state->column_nr, "!");
      ++state->column_nr;
      return 1;
      }
      case '/':
      {
      if (treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr))
        {
        return 1;
        }
      next_char(str);
      char t;
      int valid_peek = get_char(&t, str, 0); // peek
      if (valid_peek && t == '=')
        {
        *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_ASSIGNMENT_DIV, state->line_nr, state->column_nr, "/=");
        next_char(str);
        state->column_nr += 2;
        return 1;
        }
      else if (valid_peek && t == '/') // comment till end of the line
        {
        while (valid_chars_remaining && s != '\n') // comment, so skip till end of the line
          valid_chars_remaining = get_char(&s, str, 1);
        valid_chars_remaining = get_char(&s, str, 0);
        ++state->line_nr;
        state->column_nr = 1;
        break;
        }
      else if (valid_peek && t == '*') // multiline comment
        {
        valid_chars_remaining = get_char(&s, str, 1);
        valid_chars_remaining = get_char(&s, str, 1);
        ++state->column_nr;
        int end_of_comment_found = 0;
        while (!end_of_comment_found)
          {
          while (valid_chars_remaining && s != '*')
            {
            if (s == '\n')
              {
              ++state->line_nr;
              state->column_nr = 0;
              }
            valid_chars_remaining = get_char(&s, str, 1);
            ++state->column_nr;
            }
          if (!valid_chars_remaining)
            {
            end_of_comment_found = 1;
            }
          else
            {
            valid_chars_remaining = get_char(&s, str, 1);
            if (valid_chars_remaining && s == '/')
              end_of_comment_found = 1;
            }
          }
        valid_chars_remaining = get_char(&s, str, 0);
        ++state->column_nr;
        break;
        }
      *tok = cscript_make_token_cstr(ctxt, CSCRIPT_T_DIV, state->line_nr, state->column_nr, "/");
      ++state->column_nr;
      return 1;
      }
      } // switch (s)


    if ((str_position == get_position(str)) && valid_chars_remaining)
      {
      cscript_string_push_back(ctxt, buff, s);
      next_char(str);
      valid_chars_remaining = get_char(&s, str, 0);
      ++state->column_nr;
      }

    }
  return treat_buffer_token(ctxt, buff, tok, state->line_nr, state->column_nr);
  }


// read == 0 equals a peek
static int _stream_get_char(char* ch, void* st, int read)
  {
  cscript_stream* str = cast(cscript_stream*, st);
  if (str->position >= str->length)
    {
    return 0;
    }
  *ch = str->data[str->position];
  str->position += read;
  return 1;
  }

static void _stream_next_char(void* st)
  {
  cscript_stream* str = cast(cscript_stream*, st);
  ++str->position;
  }

static cscript_fixnum _stream_get_position(void* st)
  {
  cscript_stream* str = cast(cscript_stream*, st);
  return cast(cscript_fixnum, str->position);
  }

int cscript_read_token(token* tok, cscript_context* ctxt, cscript_string* buff, cscript_stream* str, cscript_read_token_state* state)
  {
  return cscript_read_token_polymorph(tok, ctxt, buff, cast(void*, str), state, &_stream_get_char, &_stream_next_char, &_stream_get_position);
  }

cscript_vector tokenize(cscript_context* ctxt, cscript_stream* str)
  {
  cscript_vector tokens;
  cscript_vector_init(ctxt, &tokens, token);

  cscript_string buff;
  cscript_string_init(ctxt, &buff, "");

  cscript_read_token_state state;
  state.line_nr = 1;
  state.column_nr = 1;

  token tok;
  while (cscript_read_token(&tok, ctxt, &buff, str, &state))
    {
    cscript_vector_push_back(ctxt, &tokens, tok, token);
    }

  cscript_string_destroy(ctxt, &buff);
  return tokens;
  }

void cscript_token_destroy(cscript_context* ctxt, token* tok)
  {
  cscript_string_destroy(ctxt, &tok->value);
  }

void destroy_tokens_vector(cscript_context* ctxt, cscript_vector* tokens)
  {
  token* it = cscript_vector_begin(tokens, token);
  token* it_end = cscript_vector_end(tokens, token);
  for (; it != it_end; ++it)
    {
    cscript_token_destroy(ctxt, it);
    }
  cscript_vector_destroy(ctxt, tokens);
  }

cscript_vector cscript_script2tokens(cscript_context* ctxt, const char* script)
  {
  cscript_stream str;
  cscript_stream_init(ctxt, &str, 10);
  cscript_memsize len = cast(cscript_memsize, strlen(script));
  cscript_stream_write(ctxt, &str, script, len, 0);
  cscript_stream_rewind(&str);
  cscript_vector tokens = tokenize(ctxt, &str);
  cscript_stream_close(ctxt, &str);
  return tokens;
  }