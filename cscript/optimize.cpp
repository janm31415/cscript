#include "optimize.h"
#include "utility.h"
#include "visitor.h"
#include <functional>
#include <map>
#include <string>
#include <stdexcept>
#include <cmath>
#include <set>

#include "defines.h"

COMPILER_BEGIN

namespace
  {

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

  value_t c_mod(values v)
    {
    if (is_i(v[0]) && is_i(v[1]))
      return i(v[0]) % i(v[1]);
    return fmod(to_d(v[0]), to_d(v[1]));
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

  value_t c_tan(values v)
    {
    return std::tan(to_d(v[0]));
    }

  value_t c_atan(values v)
    {
    return std::atan(to_d(v[0]));
    }

  value_t c_atan2(values v)
    {
    return std::atan2(to_d(v[0]), to_d(v[1]));
    }

  value_t c_pow(values v)
    {
    return std::pow(to_d(v[0]), to_d(v[1]));
    }

  value_t c_min(values v)
    {
    return std::min(to_d(v[0]), to_d(v[1]));
    }

  value_t c_max(values v)
    {
    return std::max(to_d(v[0]), to_d(v[1]));
    }

  c_funcs_t c_funcs =
    {
      {"+", c_add},
      {"-", c_sub},
      {"*", c_mul},
      {"/", c_div},
      {"%", c_mod},
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
      {"fabs", c_fabs},
      {"tan", c_tan},
      {"atan", c_atan},
      {"atan2", c_atan2},
      {"pow", c_pow},
      {"min", c_min},
      {"max", c_max}
    };

  bool test_functions_list_is_complete()
    {
    std::set<std::string> fies = get_internal_functions();
    for (const auto& fname : fies)
      {
      auto it = c_funcs.find(fname);
      if (it == c_funcs.end())
        return false;
      }
    return true;
    }

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
        if (j == 0 && is_constant(op.operands[0]) && is_constant(op.operands[1]))
          {
          auto v1 = get_constant_value(op.operands[j]);
          auto v2 = get_constant_value(op.operands[j + 1]);
          auto it = c_funcs.find(op.fops[j]);
          if (it == c_funcs.end())
            throw std::runtime_error("error: unknown function: " + op.fops[j]);
          c_func f = it->second;
          values v;
          v.reserve(2);
          v.push_back(v1.front());
          v.push_back(v2.front());
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
            v.push_back(get_constant_value(expr).front());
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
          return;
          }
        
        if ((a.op == "+=" || a.op == "-=") && is_zero(a.expr))
          {
          stm = Nop();
          return;
          }
        
        if (a.op == "*=" && is_zero(a.expr))
          {
          a.op = "=";
          return;
          }

        if (a.op == "/=" && is_one(a.expr))
          {
          stm = Nop();
          return;
          }
        }
      }
    };

  struct type_variables : public base_visitor<type_variables>
    {
    std::map<std::string, value_type> variable_to_type_map;
    std::set<std::string> variables_that_change_value;
    std::set<std::string> variables_that_are_used;

    type_variables() {}
    virtual ~type_variables() {}

    virtual void _postvisit(Int& i) 
      {
      if (!i.dims.empty())
        variable_to_type_map[i.name] = integer_array;
      else
        variable_to_type_map[i.name] = integer;
      if (!i.dims.empty())
        {
        bool constant_dims = true;
        for (const auto& d : i.dims)
          constant_dims &= is_constant(d);
        if (!constant_dims)
          variables_that_are_used.insert(i.name);
        }
      if (i.name.front() == '$') // global variable
        {
        variables_that_are_used.insert(i.name);
        variables_that_change_value.insert(i.name);
        }
      }

    virtual void _postvisit(Float& f)
      {
      if (!f.dims.empty())
        variable_to_type_map[f.name] = real_array;
      else
        variable_to_type_map[f.name] = real;
      if (!f.dims.empty())
        {
        bool constant_dims = true;
        for (const auto& d : f.dims)
          constant_dims &= is_constant(d);
        if (!constant_dims)
          variables_that_are_used.insert(f.name);
        }
      if (f.name.front() == '$') // global variable
        {
        variables_that_are_used.insert(f.name);
        variables_that_change_value.insert(f.name);
        }
      }

    virtual void _postvisit(IntParameter& i)
      {
      if (i.pointer)
        variable_to_type_map[i.name] = pointer_to_integer;
      else
        variable_to_type_map[i.name] = integer;
      variables_that_are_used.insert(i.name);
      }

    virtual void _postvisit(FloatParameter& f)
      {
      if (f.pointer)
        variable_to_type_map[f.name] = pointer_to_real;
      else
        variable_to_type_map[f.name] = real;
      variables_that_are_used.insert(f.name);
      }

    virtual void _postvisit(Assignment& a)
      {
      variables_that_change_value.insert(a.name);
      if (a.name.front() == '$') // global variable
        {
        variables_that_are_used.insert(a.name);        
        }
      }

    virtual void _postvisit(Variable& v)
      {
      variables_that_are_used.insert(v.name);
      }

    virtual void _postvisit(ArrayCall& a)
      {
      variables_that_are_used.insert(a.name);
      }

    virtual void _postvisit(Dereference& d)
      {
      variables_that_are_used.insert(d.name);
      }

    virtual void _postvisit(LValue& a)
      {
      if (std::holds_alternative<Variable>(a.lvalue))
        {
        variables_that_change_value.insert(std::get<Variable>(a.lvalue).name);
        variables_that_are_used.insert(std::get<Variable>(a.lvalue).name);
        }      
      if (std::holds_alternative<ArrayCall>(a.lvalue))
        {
        variables_that_change_value.insert(std::get<ArrayCall>(a.lvalue).name);
        variables_that_are_used.insert(std::get<ArrayCall>(a.lvalue).name);
        }
      if (std::holds_alternative<Dereference>(a.lvalue))
        {
        variables_that_change_value.insert(std::get<Dereference>(a.lvalue).name);
        variables_that_are_used.insert(std::get<Dereference>(a.lvalue).name);
        }
      }
    };

  struct remove_dead_variables : public base_visitor<remove_dead_variables>
    {
    std::set<std::string>* p_variables_that_are_used;

    remove_dead_variables() {}
    virtual ~remove_dead_variables() {}

    virtual void _postvisit(Statement& stm)
      {
      if (std::holds_alternative<Int>(stm))
        {
        Int& i = std::get<Int>(stm);
        auto it = p_variables_that_are_used->find(i.name);
        if (it == p_variables_that_are_used->end())
          {
          if (i.expr.operands.empty())
            {
            stm = Nop();
            }
          else if (is_constant(i.expr))
            {
            stm = Nop();
            }
          else
            {
            stm = i.expr;            
            }
          }
        }
      else if (std::holds_alternative<Float>(stm))
        {
        Float& f = std::get<Float>(stm);
        auto it = p_variables_that_are_used->find(f.name);
        if (it == p_variables_that_are_used->end())
          {
          if (f.expr.operands.empty())
            {
            stm = Nop();
            }
          else if (is_constant(f.expr))
            {
            stm = Nop();
            }
          else
            {
            stm = f.expr;
            }
          }
        }
      else if (std::holds_alternative<Assignment>(stm))
        {
        Assignment& a = std::get<Assignment>(stm);
        auto it = p_variables_that_are_used->find(a.name);
        if (it == p_variables_that_are_used->end())
          {
          // We don't check the dimensions because we know they are constant.
          // When we compute the variables that are used or not in visitor type_variables, if the dimensions are not constant, they are added as being used and so we cannot end up here.
          if (is_constant(a.expr))
            {
            stm = Nop();
            }
          else
            {
            stm = a.expr;
            }
          }
        }
      }
    };

  struct replace_constant_variables : public base_visitor<replace_constant_variables>
    {
    std::map<std::string, value_type>* p_variable_to_type_map;
    std::set<std::string>* p_variables_that_change_value;

    std::map<std::string, double> variable_to_value_map;

    replace_constant_variables() {}
    virtual ~replace_constant_variables() {}

    virtual void _postvisit(Factor& f)
      {
      if (std::holds_alternative<Variable>(f.factor))
        {
        auto& v = std::get<Variable>(f.factor);
        auto it = variable_to_value_map.find(v.name);
        if (it != variable_to_value_map.end())
          {
          value_t val = it->second;
          f.factor = val;
          }
        }
      }

    virtual void _postvisit(Statement& stm)
      {
      if (std::holds_alternative<Int>(stm))
        {
        Int& i = std::get<Int>(stm);
        auto it = p_variables_that_change_value->find(i.name);
        if (it == p_variables_that_change_value->end() && i.dims.empty())
          {
          if (i.expr.operands.empty())
            {
            variable_to_value_map[i.name] = 0.0;
            stm = Nop();
            }
          else if (is_constant(i.expr))
            {
            value_t v = get_constant_value(i.expr).front();
            variable_to_value_map[i.name] = to_d(v);
            stm = Nop();
            }
          }        
        }
      else if (std::holds_alternative<Float>(stm))
        {
        Float& f = std::get<Float>(stm);
        auto it = p_variables_that_change_value->find(f.name);
        if (it == p_variables_that_change_value->end() && f.dims.empty())
          {
          if (f.expr.operands.empty())
            {
            variable_to_value_map[f.name] = 0.0;
            stm = Nop();
            }
          else if (is_constant(f.expr))
            {
            value_t v = get_constant_value(f.expr).front();
            variable_to_value_map[f.name] = to_d(v);
            stm = Nop();
            }
          }        
        }
      }
    };

  struct strength_reduction : public base_visitor<strength_reduction>
    {
    std::map<std::string, value_type>* p_variable_to_type_map;

    strength_reduction() : p_variable_to_type_map(nullptr) {}
    virtual ~strength_reduction() {}

    bool named_variable_is_integer(const std::string& name)
      {
      auto it = p_variable_to_type_map->find(name);
      assert(it != p_variable_to_type_map->end());
      switch (it->second)
        {
        case real: return false;
        case integer: return true;
        case real_array: return false;
        case integer_array: return true;
        case pointer_to_real: return false;
        case pointer_to_integer: return true;
        }
      return false;
      }

    bool result_is_integer(const LValue& expr)
      {
      if (std::holds_alternative<Variable>(expr.lvalue))
        {
        return named_variable_is_integer(std::get<Variable>(expr.lvalue).name);
        }
      if (std::holds_alternative<Dereference>(expr.lvalue))
        {
        return named_variable_is_integer(std::get<Dereference>(expr.lvalue).name);
        }
      if (std::holds_alternative<ArrayCall>(expr.lvalue))
        {
        return named_variable_is_integer(std::get<ArrayCall>(expr.lvalue).name);
        }
      return false;
      }

    bool result_is_integer(const Factor& expr)
      {
      if (std::holds_alternative<value_t>(expr.factor))
        {
        auto val = std::get<value_t>(expr.factor);
        return std::holds_alternative<int64_t>(val);
        }
      if (std::holds_alternative<Variable>(expr.factor))
        {
        return named_variable_is_integer(std::get<Variable>(expr.factor).name);
        }
      if (std::holds_alternative<Dereference>(expr.factor))
        {
        return named_variable_is_integer(std::get<Dereference>(expr.factor).name);     
        }
      if (std::holds_alternative<ArrayCall>(expr.factor))
        {
        return named_variable_is_integer(std::get<ArrayCall>(expr.factor).name);
        }
      if (std::holds_alternative<LValueOperator>(expr.factor))
        {
        return result_is_integer(*std::get<LValueOperator>(expr.factor).lvalue);
        }
      return false;
      }

    bool result_is_integer(const Term& expr)
      {
      for (const auto& op : expr.operands)
        {
        if (!result_is_integer(op))
          return false;
        }
      return true;
      }

    bool result_is_integer(const Relop& expr)
      {
      for (const auto& op : expr.operands)
        {
        if (!result_is_integer(op))
          return false;        
        }
      return true;
      }
    
    virtual void _postvisit(Expression& expr)
      {
      assert(p_variable_to_type_map);
      if (expr.fops.size() == 1)
        {
        if (expr.fops[0] == "<=")
          {
          if (expr.operands[0].operands.size() == 2 && result_is_integer(expr.operands[1]))
            {
            if (expr.operands[0].fops[0] == "+")
              {
              if (is_one(expr.operands[0].operands[0])) // 1 + a <= can be replaced by a <
                {
                expr.fops[0] = "<";
                expr.operands[0].operands.erase(expr.operands[0].operands.begin());
                expr.operands[0].fops.clear();
                return;
                }
              if (is_one(expr.operands[0].operands[1])) // a + 1 <= can be replaced by a <
                {
                expr.fops[0] = "<";
                expr.operands[0].operands.pop_back();
                expr.operands[0].fops.clear();
                return;
                }
              }
            }
          if (expr.operands[1].operands.size() == 2 && result_is_integer(expr.operands[0]))
            {
            if (expr.operands[1].fops[0] == "+")
              {
              if (is_minus_one(expr.operands[1].operands[0])) // <= -1 + a can be replaced by < a
                {
                expr.fops[0] = "<";
                expr.operands[1].operands.erase(expr.operands[1].operands.begin());
                expr.operands[1].fops.clear();
                return;
                }
              }
            if (expr.operands[1].fops[0] == "-")
              {
              if (is_one(expr.operands[1].operands[1])) // <= a - 1 can be replaced by < a
                {
                expr.fops[0] = "<";
                expr.operands[1].operands.pop_back();
                expr.operands[1].fops.clear();
                return;
                }
              }
            }
          }

        if (expr.fops[0] == ">=")
          {
          if (expr.operands[0].operands.size() == 2 && result_is_integer(expr.operands[1]))
            {
            if (expr.operands[0].fops[0] == "+")
              {
              if (is_minus_one(expr.operands[0].operands[0])) // -1 + a >= can be replaced by a >
                {
                expr.fops[0] = ">";
                expr.operands[0].operands.erase(expr.operands[0].operands.begin());
                expr.operands[0].fops.clear();
                return;
                }
              }
            if (expr.operands[0].fops[0] == "-")
              {
              if (is_one(expr.operands[0].operands[1])) // a - 1 >= can be replaced by a >
                {
                expr.fops[0] = ">";
                expr.operands[0].operands.pop_back();
                expr.operands[0].fops.clear();
                return;
                }
              }
            }
          if (expr.operands[1].operands.size() == 2 && result_is_integer(expr.operands[0]))
            {
            if (expr.operands[1].fops[0] == "+")
              {
              if (is_one(expr.operands[1].operands[0])) // >= 1 + a can be replaced by > a
                {
                expr.fops[0] = ">";
                expr.operands[1].operands.erase(expr.operands[1].operands.begin());
                expr.operands[1].fops.clear();
                return;
                }
              }
            if (is_one(expr.operands[1].operands[1])) // >= a + 1 can be replaced by > a
              {
              expr.fops[0] = ">";
              expr.operands[1].operands.pop_back();
              expr.operands[1].fops.clear();
              return;
              }
            }
          }

        if (expr.fops[0] == "<")
          {
          if (expr.operands[0].operands.size() == 2 && result_is_integer(expr.operands[1]))
            {
            if (expr.operands[0].fops[0] == "+")
              {
              if (is_minus_one(expr.operands[0].operands[0])) // -1 + a < can be replaced by a <=
                {
                expr.fops[0] = "<=";
                expr.operands[0].operands.erase(expr.operands[0].operands.begin());
                expr.operands[0].fops.clear();
                return;
                }
              }
            if (expr.operands[0].fops[0] == "-")
              {
              if (is_one(expr.operands[0].operands[1])) // a - 1 < can be replaced by a <=
                {
                expr.fops[0] = "<=";
                expr.operands[0].operands.pop_back();
                expr.operands[0].fops.clear();
                return;
                }
              }
            }
          if (expr.operands[1].operands.size() == 2 && result_is_integer(expr.operands[0]))
            {
            if (expr.operands[1].fops[0] == "+")
              {
              if (is_one(expr.operands[1].operands[0])) // < 1 + a can be replaced by <= a
                {
                expr.fops[0] = "<=";
                expr.operands[1].operands.erase(expr.operands[1].operands.begin());
                expr.operands[1].fops.clear();
                return;
                }
              if (is_one(expr.operands[1].operands[1])) // < a + 1 can be replaced by <= a
                {
                expr.fops[0] = "<=";
                expr.operands[1].operands.pop_back();
                expr.operands[1].fops.clear();
                return;
                }
              }
            }
          }

        if (expr.fops[0] == ">")
          {
          if (expr.operands[0].operands.size() == 2 && result_is_integer(expr.operands[1]))
            {
            if (expr.operands[0].fops[0] == "+")
              {
              if (is_one(expr.operands[0].operands[0])) // 1 + a > can be replaced by a >=
                {
                expr.fops[0] = ">=";
                expr.operands[0].operands.erase(expr.operands[0].operands.begin());
                expr.operands[0].fops.clear();
                return;
                }
              if (is_one(expr.operands[0].operands[1])) // a + 1 > can be replaced by a >=
                {
                expr.fops[0] = ">=";
                expr.operands[0].operands.pop_back();
                expr.operands[0].fops.clear();
                return;
                }
              }
            }
          if (expr.operands[1].operands.size() == 2 && result_is_integer(expr.operands[0]))
            {
            if (expr.operands[1].fops[0] == "+")
              {
              if (is_minus_one(expr.operands[1].operands[0])) // > -1 + a can be replaced by >= a
                {
                expr.fops[0] = ">=";
                expr.operands[1].operands.erase(expr.operands[1].operands.begin());
                expr.operands[1].fops.clear();
                return;
                }
              }
            if (expr.operands[1].fops[0] == "-")
              {
              if (is_one(expr.operands[1].operands[1])) // > a - 1 can be replaced by >= a
                {
                expr.fops[0] = ">=";
                expr.operands[1].operands.pop_back();
                expr.operands[1].fops.clear();
                return;
                }
              }
            }
          }
        }

      }
    };
  } // anonymous namespace

void optimize(Program& prog)
  {
  assert(test_functions_list_is_complete());
  type_variables tv;
  visitor<Program, type_variables>::visit(prog, &tv);  
  simplify_constant_expressions sce;
  visitor<Program, simplify_constant_expressions>::visit(prog, &sce);
  remove_dead_variables rdv;
  rdv.p_variables_that_are_used = &tv.variables_that_are_used;
  visitor<Program, remove_dead_variables>::visit(prog, &rdv);
  replace_constant_variables rcv;
  rcv.p_variable_to_type_map = &tv.variable_to_type_map;
  rcv.p_variables_that_change_value = &tv.variables_that_change_value;
  visitor<Program, replace_constant_variables>::visit(prog, &rcv);
  simplify_constant_expressions sce2; // 2nd time
  visitor<Program, simplify_constant_expressions>::visit(prog, &sce2);
  simplify_assignment sa;
  visitor<Program, simplify_assignment>::visit(prog, &sa);
  strength_reduction sr;
  sr.p_variable_to_type_map = &tv.variable_to_type_map;
  visitor<Program, strength_reduction>::visit(prog, &sr);
  }

COMPILER_END
