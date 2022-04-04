#include "optimize.h"
#include "utility.h"
#include "visitor.h"
#include <functional>
#include <map>
#include <string>
#include <stdexcept>

COMPILER_BEGIN

typedef std::function<value_t(values)> c_func;
typedef std::map<std::string, c_func> c_funcs_t;


value_t c_add(values v)
  {
  if (is_i(v[0]) && is_i(v[1]))
    return i(v[0]) + i(v[1]);
  return to_d(v[0]) + to_d(v[1]);
  }

value_t c_sub(values v)
  {
  if (is_i(v[0]) && is_i(v[1]))
    return i(v[0]) - i(v[1]);
  return to_d(v[0]) - to_d(v[1]);
  }

value_t c_mul(values v)
  {
  if (is_i(v[0]) && is_i(v[1]))
    return i(v[0]) * i(v[1]);
  return to_d(v[0]) * to_d(v[1]);
  }

value_t c_div(values v)
  {
  if (is_i(v[0]) && is_i(v[1]))
    return i(v[0]) / i(v[1]);
  return to_d(v[0]) / to_d(v[1]);
  }

value_t c_equal(values v)
  {
  bool eq;
  if (is_i(v[0]) && is_i(v[1]))
    eq = i(v[0]) == i(v[1]);
  eq = to_d(v[0]) == to_d(v[1]);
  return eq ? (int64_t)1 : (int64_t)0;
  }

value_t c_notequal(values v)
  {
  bool eq;
  if (is_i(v[0]) && is_i(v[1]))
    eq = i(v[0]) != i(v[1]);
  eq = to_d(v[0]) != to_d(v[1]);
  return eq ? (int64_t)1 : (int64_t)0;
  }

value_t c_less(values v)
  {
  bool eq;
  if (is_i(v[0]) && is_i(v[1]))
    eq = i(v[0]) < i(v[1]);
  eq = to_d(v[0]) < to_d(v[1]);
  return eq ? (int64_t)1 : (int64_t)0;
  }

value_t c_leq(values v)
  {
  bool eq;
  if (is_i(v[0]) && is_i(v[1]))
    eq = i(v[0]) <= i(v[1]);
  eq = to_d(v[0]) <= to_d(v[1]);
  return eq ? (int64_t)1 : (int64_t)0;
  }

value_t c_greater(values v)
  {
  bool eq;
  if (is_i(v[0]) && is_i(v[1]))
    eq = i(v[0]) > i(v[1]);
  eq = to_d(v[0]) > to_d(v[1]);
  return eq ? (int64_t)1 : (int64_t)0;
  }

value_t c_geq(values v)
  {
  bool eq;
  if (is_i(v[0]) && is_i(v[1]))
    eq = i(v[0]) >= i(v[1]);
  eq = to_d(v[0]) >= to_d(v[1]);
  return eq ? (int64_t)1 : (int64_t)0;
  }

value_t c_sin(values v)
  {
  return std::sin(to_d(v[0]));
  }

value_t c_cos(values v)
  {
  return std::cos(to_d(v[0]));
  }

value_t c_sqrt(values v)
  {
  return std::sqrt(to_d(v[0]));
  }

value_t c_exp(values v)
  {
  return std::exp(to_d(v[0]));
  }

value_t c_log(values v)
  {
  return std::log(to_d(v[0]));
  }

value_t c_log2(values v)
  {
  return std::log2(to_d(v[0]));
  }

value_t c_fabs(values v)
  {
  return std::fabs(to_d(v[0]));
  }

c_funcs_t c_funcs =
  {
  {"+", c_add},
  {"-", c_sub},
  {"*", c_mul},
  {"/", c_div},
  {"==", c_equal},
  {"!=", c_notequal},
  {"<", c_less},
  {"<=", c_leq},
  {">", c_greater},
  {">=", c_geq},
  {"sin", c_sin},
  {"cos", c_cos},
  {"sqrt", c_sqrt},
  {"exp", c_exp},
  {"log", c_log},
  {"log2", c_log2},
  {"fabs", c_fabs}
  };

struct simplify_constant_expressions : public base_visitor<simplify_constant_expressions>
  {
  simplify_constant_expressions() {}
  virtual ~simplify_constant_expressions() {}

  template <class T>
  void _simplify(T& op)
    {
    int j = 0;
    while (j < op.fops.size())
      {
      if (is_constant(op.operands[j]) && is_constant(op.operands[j + 1]))
        {
        auto v1 = get_constant_value(op.operands[j]);
        auto v2 = get_constant_value(op.operands[j + 1]);
        auto it = c_funcs.find(op.fops[j]);
        if (it == c_funcs.end())
          throw std::runtime_error("error: unknown function: " + op.fops[j]);
        c_func f = it->second;
        values v;
        v.reserve(2);
        v.push_back(v1);
        v.push_back(v2);
        auto res = f(v);
        op.fops.erase(op.fops.begin() + j);
        op.operands.erase(op.operands.begin() + j);
        set_constant_value(op.operands[j], res);
        --j;
        }
      ++j;
      }
    }

  virtual void _postvisit(Expression& expr)
    {
    _simplify<Expression>(expr);
    }

  virtual void _postvisit(Relop& r)
    {
    _simplify<Relop>(r);
    }

  virtual void _postvisit(Term& t)
    {
    _simplify<Term>(t);
    }

  virtual void _postvisit(Factor& f)
    {
    if (is_constant(f))
      {
      if (std::holds_alternative<FuncCall>(f.factor))
        {
        auto it = c_funcs.find(std::get<FuncCall>(f.factor).name);
        if (it == c_funcs.end())
          throw std::runtime_error("error: unknown function: " + std::get<FuncCall>(f.factor).name);
        c_func fun = it->second;
        values v;
        auto& exprs = std::get<FuncCall>(f.factor).exprs;
        for (auto& expr : exprs)
          v.push_back(get_constant_value(expr));
        auto res = fun(v);      
        if (f.sign == '-')
          {
          if (std::holds_alternative<double>(res))
            res = -std::get<double>(res);
          else
            res = -std::get<int64_t>(res);
          }
        set_constant_value(f, res);
        }
      if (f.sign == '-')
        {
        auto v = get_constant_value(f);
        set_constant_value(f, v);
        }
      }
    }
  };

struct simplify_assignment : public base_visitor<simplify_assignment>
  {
  simplify_assignment() {}
  virtual ~simplify_assignment() {}

  virtual void _previsit(Assignment&)
    {
    }

  virtual void _postvisit(Assignment& a)
    {
    if (a.op == "=" && a.expr.operands.size() == 1)
      {
      Relop& r = a.expr.operands.front();
      if (r.operands.size() > 1) // + or -
        {
        int j = 0;
        while (j < r.operands.size())
          {
          if (a.dereference)
            {
            Dereference d;
            d.name = a.name;
            if (equals(d, r.operands[j]))
              break;
            }
          else if (a.dims.empty())
            {
            Variable v;
            v.name = a.name;
            if (equals(v, r.operands[j]))
              break;
            }
          else
            {
            ArrayCall ac;
            ac.name = a.name;
            ac.exprs = a.dims;
            if (equals(ac, r.operands[j]))
              break;
            }
          ++j;
          } // while (j ...
        if (j < r.operands.size())
          {
          bool neg = false;
          if (j > 0 && r.fops[j - 1] == "-")
            neg = true;
          if (!neg)
            {
            if (j < r.fops.size() && r.fops[j] == "-")
              a.op = "-=";
            else
              a.op = "+=";
            r.operands.erase(r.operands.begin() + j);
            if (j)
              r.fops.erase(r.fops.begin() + j - 1);
            else
              r.fops.erase(r.fops.begin());
            }
          }
        }
      else if (r.operands.size() == 1 && r.operands.front().operands.size() > 1) // * or /
        {
        Term& t = r.operands.front();
        int j = 0;
        while (j < t.operands.size())
          {
          if (a.dims.empty())
            {
            Variable v;
            v.name = a.name;
            if (equals(v, t.operands[j]))
              break;
            }
          else
            {
            ArrayCall ac;
            ac.name = a.name;
            ac.exprs = a.dims;
            if (equals(ac, t.operands[j]))
              break;
            }
          ++j;
          } // while (j ...
        if (j < t.operands.size())
          {
          bool div = false;
          if (j > 0 && t.fops[j - 1] == "/")
            div = true;
          if (!div)
            {
            if (j < t.fops.size() && t.fops[j] == "/")
              a.op = "/=";
            else
              a.op = "*=";
            t.operands.erase(t.operands.begin() + j);
            if (j)
              t.fops.erase(t.fops.begin() + j - 1);
            else
              t.fops.erase(t.fops.begin());
            }
          }
        } // * or /
      }
    }

  virtual void _postvisit(Statement& stm)
    {
    if (std::holds_alternative<Assignment>(stm))
      {
      Assignment& a = std::get<Assignment>(stm);
      if ((a.op == "+=" || a.op == "-=") && is_one(a.expr))
        {
        Expression expr;
        Relop r;
        Term t;
        Factor f;
        LValueOperator lvo;
        LValue lv;
        if (a.dereference)
          {
          Dereference d;
          d.name = a.name;
          lv.lvalue = d;
          }
        else if (a.dims.empty())
          {
          Variable v;
          v.name = a.name;
          lv.lvalue = v;
          }
        else
          {
          ArrayCall ac;
          ac.name = a.name;
          ac.exprs = a.dims;
          lv.lvalue = ac;
          }
        lvo.name = a.op == "+=" ? "++" : "--";
        lvo.lvalue = std::make_shared<LValue>(lv);
        f.factor = lvo;
        t.operands.push_back(f);
        r.operands.push_back(t);
        expr.operands.push_back(r);
        stm = expr;
        }
      }
    }
  };

void optimize(Program& prog)
  {
  simplify_constant_expressions sce;
  visitor<Program, simplify_constant_expressions>::visit(prog, &sce);
  simplify_assignment sa;
  visitor<Program, simplify_assignment>::visit(prog, &sa);
  }

COMPILER_END