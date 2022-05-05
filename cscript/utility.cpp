#include "utility.h"

COMPILER_BEGIN


double d(value_t v)
  {
  return std::get<double>(v);
  }

int64_t i(value_t v)
  {
  return std::get<int64_t>(v);
  }

bool is_d(value_t v)
  {
  return std::holds_alternative<double>(v);
  }

bool is_i(value_t v)
  {
  return std::holds_alternative<int64_t>(v);
  }

double to_d(value_t v)
  {
  if (is_i(v))
    return (double)i(v);
  return d(v);
  }

int64_t to_i(value_t v)
  {
  if (is_d(v))
    return (int64_t)d(v);
  return i(v);
  }

bool is_empty(const Expression& expr)
  {
  return expr.operands.empty();
  }

bool is_constant(const Factor& f)
  {
  return (std::holds_alternative<value_t>(f.factor)
    || (std::holds_alternative<Expression>(f.factor) && is_constant(std::get<Expression>(f.factor)))
    || (std::holds_alternative<FuncCall>(f.factor) && is_constant(std::get<FuncCall>(f.factor).exprs))
    || (std::holds_alternative<ExpressionList>(f.factor) && is_constant(std::get<ExpressionList>(f.factor).exprs))
    );
  }

bool is_constant(const Term& t)
  {
  return (t.operands.size() == 1) && is_constant(t.operands.front());
  }

bool is_constant(const Relop& r)
  {
  return (r.operands.size() == 1) && is_constant(r.operands.front());
  }

bool is_constant(const Expression& expr)
  {
  return (expr.operands.size() == 1) && is_constant(expr.operands.front());
  }

bool is_constant(const std::vector<Expression>& exprs)
  {
  for (auto expr : exprs)
    if (!is_constant(expr))
      return false;
  return true;
  }

std::vector<value_t> get_constant_value(const Factor& f)
  {
  std::vector<value_t> ret;
  if (std::holds_alternative<value_t>(f.factor))
    ret.push_back(std::get<value_t>(f.factor));
  else if (std::holds_alternative<ExpressionList>(f.factor))
    {
    for (const auto& expr : std::get<ExpressionList>(f.factor).exprs)
      {
      auto values = get_constant_value(expr);
      ret.insert(ret.end(), values.begin(), values.end());
      }
    }
  else
    ret = get_constant_value(std::get<Expression>(f.factor));
  if (f.sign == '-')
    {
    for (auto& r : ret)
      {
      if (std::holds_alternative<double>(r))
        r = -std::get<double>(r);
      else
        r = -std::get<int64_t>(r);
      }
    }
  return ret;
  }

std::vector<value_t> get_constant_value(const Term& t)
  {
  return get_constant_value(t.operands.front());
  }

std::vector<value_t> get_constant_value(const Relop& r)
  {
  return get_constant_value(r.operands.front());
  }

std::vector<value_t> get_constant_value(const Expression& expr)
  {
  return get_constant_value(expr.operands.front());
  }

void set_constant_value(Factor& f, const std::vector<value_t>& v)
  {
  if (v.size() == 1)
    set_constant_value(f, v.front());
  else
    {
    f.sign = '+';
    ExpressionList expr;
    for (const auto& val : v)
      {
      Expression e;
      Factor ff;
      ff.factor = val;
      ff.sign = '+';
      Term tt;
      tt.operands.push_back(ff);
      Relop rr;
      rr.operands.push_back(tt);
      e.operands.push_back(rr);
      expr.exprs.push_back(e);
      }
    f.factor = expr;
    }
  }

void set_constant_value(Factor& f, value_t v)
  {
  f.sign = '+';
  f.factor = v;
  }

void set_constant_value(Term& t, value_t v)
  {
  set_constant_value(t.operands.front(), v);
  }

void set_constant_value(Relop& r, value_t v)
  {
  set_constant_value(r.operands.front(), v);
  }

void set_constant_value(Expression& expr, value_t v)
  {
  set_constant_value(expr.operands.front(), v);
  }

bool equals(const Variable& v, const Term& t)
  {
  return (t.operands.size() == 1) && equals(v, t.operands.front());
  }

bool equals(const Variable& v, const Factor& f)
  {
  if (std::holds_alternative<Variable>(f.factor))
    {
    return std::get<Variable>(f.factor).name == v.name;
    }
  return false;
  }

bool equals(const Dereference& v, const Term& t)
  {
  return (t.operands.size() == 1) && equals(v, t.operands.front());
  }

bool equals(const Dereference& v, const Factor& f)
  {
  if (std::holds_alternative<Dereference>(f.factor))
    {
    return std::get<Dereference>(f.factor).name == v.name;
    }
  return false;
  }

bool equals(const ArrayCall& v, const Term& t)
  {
  return (t.operands.size() == 1) && equals(v, t.operands.front());
  }

bool equals(const ArrayCall& v, const Factor& f)
  {
  if (std::holds_alternative<ArrayCall>(f.factor))
    {
    if (std::get<ArrayCall>(f.factor).name != v.name)
      return false;
    if (v.exprs.size() != std::get<ArrayCall>(f.factor).exprs.size())
      return false;
    for (size_t j = 0; j < v.exprs.size(); ++j)
      if (!equals(v.exprs[j], std::get<ArrayCall>(f.factor).exprs[j]))
        return false;
    return true;
    }
  return false;
  }

template <class T>
bool _equals(const T& left, const T& right)
  {
  if (left.operands.size() != right.operands.size())
    return false;
  if (left.fops.size() != right.fops.size())
    return false;
  for (size_t j = 0; j < left.fops.size(); ++j)
    if (left.fops[j] != right.fops[j])
      return false;
  for (size_t j = 0; j < left.operands.size(); ++j)
    if (!equals(left.operands[j], right.operands[j]))
      return false;
  return true;
  }

bool equals(const Expression& left, const Expression& right)
  {
  return _equals<Expression>(left, right);
  }

bool equals(const Relop& left, const Relop& right)
  {
  return _equals<Relop>(left, right);
  }

bool equals(const Term& left, const Term& right)
  {
  return _equals<Term>(left, right);
  }

bool equals(const Factor& left, const Factor& right)
  {
  if (left.factor.index() != right.factor.index())
    return false;
  if (std::holds_alternative<value_t>(left.factor))
    {
    return equals(std::get<value_t>(left.factor), std::get<value_t>(right.factor));
    }
  else if (std::holds_alternative<Expression>(left.factor))
    {
    return equals(std::get<Expression>(left.factor), std::get<Expression>(right.factor));
    }
  return false;
  }

bool equals(const value_t& left, const value_t& right)
  {
  return to_d(left) == to_d(right);
  }

bool is_one(const Expression& expr)
  {
  return (expr.operands.size() == 1) && is_one(expr.operands.front());
  }

bool is_one(const Relop& expr)
  {
  return (expr.operands.size() == 1) && is_one(expr.operands.front());
  }

bool is_one(const Term& expr)
  {
  return (expr.operands.size() == 1) && is_one(expr.operands.front());
  }

bool is_one(const Factor& expr)
  {
  if (std::holds_alternative<value_t>(expr.factor))
    return is_one(std::get<value_t>(expr.factor));
  return false;
  }

bool is_one(const value_t& expr)
  {
  return to_d(expr) == 1.0;
  }

bool is_minus_one(const Expression& expr)
  {
  return (expr.operands.size() == 1) && is_minus_one(expr.operands.front());
  }

bool is_minus_one(const Relop& expr)
  {
  return (expr.operands.size() == 1) && is_minus_one(expr.operands.front());
  }

bool is_minus_one(const Term& expr)
  {
  return (expr.operands.size() == 1) && is_minus_one(expr.operands.front());
  }

bool is_minus_one(const Factor& expr)
  {
  if (std::holds_alternative<value_t>(expr.factor))
    return is_minus_one(std::get<value_t>(expr.factor));
  return false;
  }

bool is_minus_one(const value_t& expr)
  {
  return to_d(expr) == -1.0;
  }


bool is_zero(const Expression& expr)
  {
  return (expr.operands.size() == 1) && is_zero(expr.operands.front());
  }

bool is_zero(const Relop& expr)
  {
  return (expr.operands.size() == 1) && is_zero(expr.operands.front());
  }

bool is_zero(const Term& expr)
  {
  return (expr.operands.size() == 1) && is_zero(expr.operands.front());
  }

bool is_zero(const Factor& expr)
  {
  if (std::holds_alternative<value_t>(expr.factor))
    return is_zero(std::get<value_t>(expr.factor));
  return false;
  }

bool is_zero(const value_t& expr)
  {
  return to_d(expr) == 0.0;
  }

bool is_simple_relative_operation(const Statement& stm)
  {
  return std::holds_alternative<Expression>(stm) && (std::get<Expression>(stm).fops.size() == 1);
  }

COMPILER_END