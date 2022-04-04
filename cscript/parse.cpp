#include "parse.h"
#include "tokenize.h"
#include <stdio.h>
#include <inttypes.h>
#include <sstream>
#include <map>
#include <functional>

COMPILER_BEGIN

Statement make_statement(tokens& tokes);
Statements make_statements(tokens& tokes, std::string terminator);
Expression make_expression(tokens& tokes);
LValueOperator make_increment_decrement_call(const std::string& name, tokens& tokes);

namespace
  {
  int64_t s64(const char *s)
    {
    uint64_t i;
    char c;
    sscanf(s, "%" SCNu64 "%c", &i, &c);
    return (int64_t)i;
    }

  void throw_parse_error(int line_nr, const std::string& message)
    {
    if (line_nr <= 0)
      throw std::logic_error("parse error: " + message);
    std::stringstream str;
    str << line_nr;
    throw std::logic_error("parse error: line " + str.str() + ": " + message);
    }

  std::string current(const tokens& tokens)
    {
    return tokens.empty() ? std::string() : tokens.back().value;
    }

  token take(tokens& tokens)
    {
    if (tokens.empty())
      {
      throw_parse_error(-1, "unexpected end");
      }
    token t = tokens.back();
    tokens.pop_back();
    return t;
    }

  token::e_type current_type(const tokens& tokens)
    {
    return tokens.empty() ? token::T_BAD : tokens.back().type;
    }

  void advance(tokens& tokens)
    {
    tokens.pop_back();
    }

  void require(tokens& tokens, std::string required)
    {
    if (tokens.empty())
      {
      throw_parse_error(-1, "unexpected end: missing " + required);
      }
    auto t = take(tokens);
    if (t.value != required)
      {
      throw_parse_error(t.line_nr, "required: " + required + ", found: " + t.value);
      }
    }

  int current_line(const tokens& tokens)
    {
    if (tokens.empty())
      throw_parse_error(-1, "unexpected end");
    return tokens.back().line_nr;
    }

  void check_for_semicolon(tokens& tokes, const Statement& stmt)
    {
    bool optional = false;
    if (std::holds_alternative<For>(stmt))
      optional = true;
    if (optional)
      {
      if (!tokes.empty() && current(tokes) == ";")
        advance(tokes);
      }
    else
      require(tokes, ";");
    }

  bool is_assignment(token::e_type t)
    {
    return t >= token::T_ASSIGNMENT && t <= token::T_ASSIGNMENT_DIV;
    }

  }

ArrayCall make_array_call(const std::string& name, tokens& tokes)
  {
  ArrayCall fn;
  fn.name = name;
  fn.line_nr = current_line(tokes);
  require(tokes, "[");
  fn.exprs.push_back(make_expression(tokes));
  require(tokes, "]");
  if (current_type(tokes) == token::T_LEFT_SQUARE_BRACKET)
    throw_parse_error(fn.line_nr, "only single dimension arrays are allowed");
  return fn;
  }

Dereference make_dereference(tokens& tokes)
  {
  Dereference de;
  de.line_nr = current_line(tokes);
  auto toke = take(tokes);
  if (toke.type != token::T_ID)
    throw_parse_error(de.line_nr, "dereferencing error");
  de.name = toke.value;
  return de;
  }

Variable make_variable(const token& toke)
  {
  Variable var;
  var.name = toke.value;
  var.line_nr = toke.line_nr;
  return var;
  }

LValue make_lvalue(tokens& tokes)
  {
  token toke = take(tokes);
  switch (toke.type)
    {
    case token::T_LEFT_ROUND_BRACKET:
    {
    LValue l = make_lvalue(tokes);
    l.line_nr = toke.line_nr;
    require(tokes, ")");
    return l;
    }
    case token::T_ID:
    {
    if (current(tokes) == "[")
      {
      LValue l;
      l.lvalue = make_array_call(toke.value, tokes);
      l.line_nr = toke.line_nr;
      return l;
      }
    else
      {
      LValue l;
      l.lvalue = make_variable(toke);
      l.line_nr = toke.line_nr;
      return l;
      }
    }
    case token::T_MUL:
    {
    LValue l;
    l.lvalue = make_dereference(tokes);
    l.line_nr = toke.line_nr;
    return l;

    }
    default:
      throw_parse_error(toke.line_nr, "unhandled type: " + toke.value);
    }
  return LValue(); // suppress warning
  }

LValueOperator make_increment_decrement_call(const std::string& name, tokens& tokes)
  {
  LValueOperator fn;
  fn.name = name;
  fn.line_nr = current_line(tokes);
  fn.lvalue = std::make_shared<LValue>(make_lvalue(tokes));
  //fn.lvalue.reset(new LValue(make_lvalue(tokes)));
  return fn;
  }

For make_for(tokens& tokes)
  {
  For f;
  f.line_nr = tokes.back().line_nr;
  require(tokes, "for");
  require(tokes, "(");
  f.init_cond_inc.push_back(make_statement(tokes));
  require(tokes, ";");
  f.init_cond_inc.push_back(make_statement(tokes));
  require(tokes, ";");
  f.init_cond_inc.push_back(make_statement(tokes));
  require(tokes, ")");
  require(tokes, "{");
  f.statements = make_statements(tokes, "}");
  return f;
  }

Int make_int(tokens& tokes)
  {
  Int i;
  i.line_nr = current_line(tokes);
  require(tokes, "int");
  i.name = take(tokes).value;
  if (current_type(tokes) == token::T_LEFT_SQUARE_BRACKET)
    {
    advance(tokes);
    i.dims.push_back(make_expression(tokes));
    require(tokes, "]");
    }
  if (current_type(tokes) == token::T_LEFT_SQUARE_BRACKET)
    throw_parse_error(i.line_nr, "only single dimension arrays are allowed");
  if (current(tokes) == "=")
    {
    advance(tokes);
    i.expr = make_expression(tokes);
    }
  return i;
  }

Float make_float(tokens& tokes)
  {
  Float f;
  f.line_nr = current_line(tokes);
  require(tokes, "float");
  f.name = take(tokes).value;
  if (current_type(tokes) == token::T_LEFT_SQUARE_BRACKET)
    {
    advance(tokes);
    f.dims.push_back(make_expression(tokes));
    require(tokes, "]");
    }
  if (current_type(tokes) == token::T_LEFT_SQUARE_BRACKET)
    throw_parse_error(f.line_nr, "only single dimension arrays are allowed");
  if (current(tokes) == "=")
    {
    advance(tokes);
    f.expr = make_expression(tokes);
    }
  return f;
  }


Assignment make_assignment(tokens& tokes)
  {
  Assignment a;
  a.line_nr = current_line(tokes);
  a.dereference = (current(tokes) == "*");
  if (a.dereference)
    tokes.pop_back();
  a.name = take(tokes).value;
  if (current_type(tokes) == token::T_LEFT_SQUARE_BRACKET)
    {
    if (a.dereference)
      throw_parse_error(a.line_nr, "cannot dereference array");
    advance(tokes);
    a.dims.push_back(make_expression(tokes));
    require(tokes, "]");
    }
  if (current_type(tokes) == token::T_LEFT_SQUARE_BRACKET)
    throw_parse_error(a.line_nr, "only single dimension arrays are allowed");
  a.op = take(tokes).value;
  a.expr = make_expression(tokes);
  return a;
  }

template<typename T>
void make_funcargs(tokens& tokes, T make)
  {
  require(tokes, "(");
  if (current(tokes) != ")")
    {
    make(tokes);
    while (current(tokes) == ",")
      {
      advance(tokes);
      make(tokes);
      }
    }
  require(tokes, ")");
  }

FuncCall make_funccall(std::string name, tokens& tokes)
  {
  FuncCall fn;
  fn.name = name;
  fn.line_nr = current_line(tokes);
  auto make = [&fn](tokens& tokes) { fn.exprs.push_back(make_expression(tokes)); };
  make_funcargs(tokes, make);
  return fn;
  }

Factor make_factor(tokens& tokes)
  {
  Factor f;
  token toke = take(tokes);
  f.line_nr = toke.line_nr;

  // optional sign
  if ((toke.value == "+") || (toke.value == "-"))
    {
    f.sign = toke.value[0];
    toke = take(tokes);
    }

  switch (toke.type) {
    case token::T_LEFT_ROUND_BRACKET:
      f.factor = make_expression(tokes);
      require(tokes, ")");
      break;
    case token::T_REAL:
      f.factor = value_t(to_double(toke.value.c_str()));
      break;
    case token::T_INTEGER:
      f.factor = value_t(s64(toke.value.c_str()));
      break;
    case token::T_INCREMENT:
    case token::T_DECREMENT:
      f.factor = make_increment_decrement_call(toke.value, tokes);
      break;
    case token::T_MUL:
      f.factor = make_dereference(tokes);
      break;
    case token::T_ID:
      if (current_type(tokes) == token::T_LEFT_ROUND_BRACKET)
        f.factor = make_funccall(toke.value, tokes);
      else if (current_type(tokes) == token::T_LEFT_SQUARE_BRACKET)
        {
        f.factor = make_array_call(toke.value, tokes);
        }
      else
        f.factor = make_variable(toke);
      break;
    default:
      throw_parse_error(toke.line_nr, "Unhandled type");
    }
  return f;
  }

template <typename T, typename U>
T parse_multiop(tokens& tokes, std::function<U(tokens&)> make, std::vector<std::string> ops)
  {
  T node;
  node.line_nr = current_line(tokes);
  node.operands.push_back(make(tokes));
  while (1) {
    std::string op = current(tokes);
    std::vector<std::string>::iterator opit = std::find(ops.begin(), ops.end(), op);
    if (opit == ops.end())
      break;
    tokes.pop_back();
    node.fops.push_back(op);
    node.operands.push_back(make(tokes));
    }
  return node;
  }

Term make_term(tokens& tokes) { return parse_multiop<Term, Factor>(tokes, make_factor, { "*", "/" }); }
Relop make_relop(tokens& tokes) { return parse_multiop<Relop, Term>(tokes, make_term, { "+", "-" }); }

Expression make_expression(tokens& tokes)
  {
  return parse_multiop<Expression, Relop>(tokes, make_relop, { ">", ">=", "<", "<=", "==", "!=" });
  }

Statement make_statement(tokens& tokes)
  {
  if (tokes.empty())
    return Nop();
  static const std::map<std::string, std::function<Statement(tokens&)>> commands =
    {
    {"for",   make_for},
    {"int",   make_int},
    {"float", make_float}
    };

  Statement stm;
  switch (tokes.back().type)
    {
    case token::T_RIGHT_ROUND_BRACKET:
    {
    return Nop();
    }
    case token::T_ID:
    {
    auto it = commands.find(current(tokes));
    if (it != commands.end())
      {
      auto make = it->second;
      stm = make(tokes);
      }
    else
      {
      if (tokes.size() >= 3)
        {
        if (is_assignment(tokes[tokes.size() - 2].type))
          {
          stm = make_assignment(tokes);
          }
        else if (tokes[tokes.size() - 2].type == token::T_LEFT_SQUARE_BRACKET)
          {
          size_t j = tokes.size() - 2;
          while (tokes[j].type == token::T_LEFT_SQUARE_BRACKET)
            {
            while (j > 0 && tokes[j].type != token::T_RIGHT_SQUARE_BRACKET)
              --j;
            if (j == 0)
              throw_parse_error(tokes.back().line_nr, "[,] mismatch");
            --j;
            }
          if (j > 0 && is_assignment(tokes[j].type))
            {
            stm = make_assignment(tokes);
            }
          else
            stm = make_expression(tokes);
          }
        else
          stm = make_expression(tokes);
        }
      else
        stm = make_expression(tokes);
      }
    break;
    }
    case token::T_MUL:
    {
    if ((tokes.size() >= 4) && is_assignment(tokes[tokes.size() - 3].type))
      {
      stm = make_assignment(tokes);
      }
    else
      stm = make_expression(tokes);
    break;
    }
    default:
    {
    stm = make_expression(tokes);
    break;
    }
    };
  return stm;
  }


Parameters make_parameters(tokens& tokes)
  {
  Parameters pars;
  if (tokes.empty())
    return pars;
  if (current(tokes) == "(")
    {
    advance(tokes);
    bool read_parameter = true;
    while (read_parameter)
      {
      auto toke = take(tokes);
      if (toke.type == token::T_RIGHT_ROUND_BRACKET)
        {
        return pars; // no parameters
        }
      if (toke.type != token::T_ID)
        throw_parse_error(toke.line_nr, "invalid parameter declaration");
      Parameter par;
      switch (toke.value.front())
        {
        case 'f':
          if (toke.value == "float")
            {
            FloatParameter fp;
            fp.line_nr = toke.line_nr;
            if (current_type(tokes) == token::T_MUL)
              {
              fp.pointer = true;
              advance(tokes);
              }
            else
              fp.pointer = false;
            toke = take(tokes);
            if (toke.type != token::T_ID)
              throw_parse_error(toke.line_nr, "compiler error: invalid parameter name");
            fp.name = toke.value;
            par = fp;
            break;
            }
        case 'i':
          if (toke.value == "int")
            {
            IntParameter ip;
            ip.line_nr = toke.line_nr;
            if (current_type(tokes) == token::T_MUL)
              {
              ip.pointer = true;
              advance(tokes);
              }
            else
              ip.pointer = false;
            toke = take(tokes);
            if (toke.type != token::T_ID)
              throw_parse_error(toke.line_nr, "compiler error: invalid parameter name");
            ip.name = toke.value;
            par = ip;
            break;
            }
        default: throw_parse_error(toke.line_nr, "invalid parameter declaration");
        }
      pars.push_back(par);
      if (current(tokes) == ")")
        read_parameter = false;
      else
        require(tokes, ",");
      }
    require(tokes, ")");
    }
  return pars;
  }

Statements make_statements(tokens& tokes, std::string terminator)
  {
  Statements stmts;
  while (current(tokes) != terminator)
    {
    stmts.push_back(make_statement(tokes));
    check_for_semicolon(tokes, stmts.back());
    }
  require(tokes, terminator);
  return stmts;
  }

Program make_program(tokens& tokes)
  {
  std::reverse(tokes.begin(), tokes.end());
  Program prog;
  prog.parameters = make_parameters(tokes);
  while (!tokes.empty())
    {
    prog.statements.push_back(make_statement(tokes));
    check_for_semicolon(tokes, prog.statements.back());
    }
  return prog;
  }

COMPILER_END