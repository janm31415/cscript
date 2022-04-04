#pragma once

#include "namespace.h"
#include <vector>
#include <stdint.h>
#include <cassert>
#include <variant>
#include <map>
#include <memory>
#include <stdint.h>

#include "cscript_api.h"
#include "tokenize.h"

COMPILER_BEGIN

typedef std::variant<double, int64_t> value_t;
typedef std::vector<value_t> values;

class Factor;
class For;
class FuncCall;
class ArrayCall;
class Dereference;
class LValue;
class LValueOperator;
class Int;
class Float;

template<typename T>
class Precedence { public: std::vector<T> operands; std::vector<std::string> fops; int line_nr; };


typedef Precedence<Factor> Term;
typedef Precedence<Term> Relop;
typedef Precedence<Relop> Expression;


class Variable { public: std::string name; int line_nr; };
class Int { public: std::string name; Expression expr; std::vector<Expression> dims; int line_nr; };
class Float { public: std::string name; Expression expr; std::vector<Expression> dims; int line_nr; };
class Assignment { public: std::string name; std::string op; Expression expr; std::vector<Expression> dims; bool dereference; int line_nr; };
class ArrayCall { public: std::string name; std::vector<Expression> exprs; int line_nr; };
class Dereference { public: std::string name; int line_nr; };
class LValueOperator { public: std::string name; std::shared_ptr<LValue> lvalue;  int line_nr; };
class LValue { public: std::variant<Variable, ArrayCall, Dereference> lvalue; int line_nr; };
class Nop {};
typedef std::variant<Nop, Expression, For, Int, Float, Assignment> Statement;
typedef std::vector<Statement> Statements;
class FuncCall { public: std::string name; std::vector<Expression> exprs; int line_nr; };
class Factor { public: char sign = '+'; std::variant<value_t, Expression, FuncCall, Variable, ArrayCall, Dereference, LValueOperator> factor; int line_nr; };
class For { public: Statements init_cond_inc; Statements statements; int line_nr; };
class IntParameter { public: std::string name; bool pointer; int line_nr; };
class FloatParameter { public: std::string name; bool pointer; int line_nr; };
typedef std::variant<IntParameter, FloatParameter> Parameter;
typedef std::vector<Parameter> Parameters;
class Program { public: Parameters parameters; Statements statements; };

COMPILER_CSCRIPT_API Program make_program(tokens& tokes);

COMPILER_END