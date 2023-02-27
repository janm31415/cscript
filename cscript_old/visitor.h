#pragma once

#include "namespace.h"
#include <variant>


#include "parse.h"

COMPILER_BEGIN


template <class U, class T>
struct visitor
  {
  static void visit(U& u, T* v)
    {
    }
  };

template <class T>
struct visitor<LValue, T>
  {
  static void visit(LValue& lv, T* v)
    {
    ((typename T::base*)v)->_previsit(lv);
    std::visit(*v, lv.lvalue);
    ((typename T::base*)v)->_postvisit(lv);
    }
  };

template <class T>
struct visitor<Factor, T>
  {
  static void visit(Factor& f, T* v)
    {
    ((typename T::base*)v)->_previsit(f);
    std::visit(*v, f.factor);
    ((typename T::base*)v)->_postvisit(f);
    }
  };

template <class T>
struct visitor<Term, T>
  {
  static void visit(Term& t, T* v)
    {
    ((typename T::base*)v)->_previsit(t);
    for (auto& op : t.operands)
      visitor<Factor, T>::visit(op, v);
    ((typename T::base*)v)->_postvisit(t);
    }
  };

template <class T>
struct visitor<Relop, T>
  {
  static void visit(Relop& relop, T* v)
    {
    ((typename T::base*)v)->_previsit(relop);
    for (auto& op : relop.operands)
      visitor<Term, T>::visit(op, v);
    ((typename T::base*)v)->_postvisit(relop);
    }
  };

template <class T>
struct visitor<Expression, T>
  {
  static void visit(Expression& expr, T* v)
    {
    //((typename T::base*)v)->_previsit(expr);
    for (auto& op : expr.operands)
      visitor<Relop, T>::visit(op, v);
    //((typename T::base*)v)->_postvisit(expr);
    }
  };

template <class T>
struct visitor<Statement, T>
  {
  static void visit(Statement& stm, T* v)
    {
    ((typename T::base*)v)->_previsit(stm);
    std::visit(*v, stm);
    ((typename T::base*)v)->_postvisit(stm);
    }
  };

template <class T>
struct visitor<Statements, T>
  {
  static void visit(Statements& stms, T* v)
    {
    for (auto& stm : stms)
      visitor<Statement, T>::visit(stm, v);
    }
  };

template <class T>
struct visitor<Parameter, T>
  {
  static void visit(Parameter& p, T* v)
    {
    std::visit(*v, p);
    }
  };

template <class T>
struct visitor<Parameters, T>
  {
  static void visit(Parameters& pars, T* v)
    {
    for (auto& p : pars)
      visitor<Parameter, T>::visit(p, v);
    }
  };

template <class T>
struct visitor<Program, T>
  {
  static void visit(Program& prog, T* v)
    {
    visitor<Parameters, T>::visit(prog.parameters, v);
    visitor<Statements, T>::visit(prog.statements, v);
    }
  };


template <class T>
struct base_visitor
  {
  typedef base_visitor<T> base;
  void operator()(Nop& i)
    {
    _previsit(i);
    _postvisit(i);
    }
  void operator()(Expression& i)
    {
    _previsit(i);
    visitor<Expression, T>::visit(i, (T*)this);
    _postvisit(i);
    }
  void operator()(For& i)
    {
    _previsit(i);
    visitor<Statements, T>::visit(i.init_cond_inc, (T*)this);
    visitor<Statements, T>::visit(i.statements, (T*)this);
    _postvisit(i);
    }
  void operator()(If& i)
    {
    _previsit(i);
    visitor<Statements, T>::visit(i.condition, (T*)this);
    visitor<Statements, T>::visit(i.body, (T*)this);
    visitor<Statements, T>::visit(i.alternative, (T*)this);
    _postvisit(i);
    }
  void operator()(Int& i)
    {
    _previsit(i);
    operator ()(i.expr);
    for (auto& dim : i.dims)
      operator ()(dim);
    _postvisit(i);
    }

  void operator()(Float& i)
    {
    _previsit(i);
    operator ()(i.expr);
    for (auto& dim : i.dims)
      operator ()(dim);
    _postvisit(i);
    }
  void operator()(IntParameter& i)
    {
    _previsit(i);   
    _postvisit(i);
    }
  void operator()(FloatParameter& i)
    {
    _previsit(i);
    _postvisit(i);
    }
  void operator()(Assignment& i)
    {
    _previsit(i);
    operator ()(i.expr);
    for (auto& dim : i.dims)
      operator ()(dim);
    _postvisit(i);
    }
  void operator()(CommaSeparatedStatements& i)
    {
    _previsit(i);
    visitor<Statements, T>::visit(i.statements, (T*)this);
    _postvisit(i);
    }

  void operator()(value_t& i)
    {
    _previsit(i);
    std::visit(*(T*)this, i);
    _postvisit(i);
    }

  void operator()(FuncCall& i)
    {
    _previsit(i);
    for (auto& e : i.exprs)
      operator ()(e);
    _postvisit(i);
    }

  void operator()(ExpressionList& i)
    {
    _previsit(i);
    for (auto& e : i.exprs)
      operator ()(e);
    _postvisit(i);
    }

  void operator()(Variable& i)
    {
    _previsit(i);
    _postvisit(i);
    }

  void operator()(ArrayCall& i)
    {
    _previsit(i);
    for (auto& e : i.exprs)
      operator ()(e);
    _postvisit(i);
    }

  void operator()(Dereference& i)
    {
    _previsit(i);
    _postvisit(i);
    }

  void operator()(LValueOperator& i)
    {
    _previsit(i);
    visitor<LValue, T>::visit(*i.lvalue.get(), (T*)this);
    _postvisit(i);
    }

  void operator()(double& i)
    {
    _previsit(i);
    _postvisit(i);
    }

  void operator()(int64_t& i)
    {
    _previsit(i);
    _postvisit(i);
    }

  virtual void _previsit(Statement&) {}
  virtual void _previsit(Nop&) {}
  virtual void _previsit(Expression&) {}
  virtual void _previsit(Relop&) {}
  virtual void _previsit(Term&) {}
  virtual void _previsit(Factor&) {}
  virtual void _previsit(LValue&) {}
  virtual void _previsit(For&) {}
  virtual void _previsit(If&) {}
  virtual void _previsit(Int&) {}
  virtual void _previsit(Float&) {}
  virtual void _previsit(IntParameter&) {}
  virtual void _previsit(FloatParameter&) {}
  virtual void _previsit(Assignment&) {}
  virtual void _previsit(CommaSeparatedStatements&) {}
  virtual void _previsit(value_t&) {}
  virtual void _previsit(FuncCall&) {}
  virtual void _previsit(ExpressionList&) {}
  virtual void _previsit(Variable&) {}
  virtual void _previsit(ArrayCall&) {}
  virtual void _previsit(Dereference&) {}
  virtual void _previsit(LValueOperator&) {}
  virtual void _previsit(double&) {}
  virtual void _previsit(int64_t&) {}

  virtual void _postvisit(Statement&) {}
  virtual void _postvisit(Nop&) {}
  virtual void _postvisit(Expression&) {}
  virtual void _postvisit(Relop&) {}
  virtual void _postvisit(Term&) {}
  virtual void _postvisit(Factor&) {}
  virtual void _postvisit(LValue&) {}
  virtual void _postvisit(For&) {}
  virtual void _postvisit(If&) {}
  virtual void _postvisit(Int&) {}
  virtual void _postvisit(Float&) {}
  virtual void _postvisit(IntParameter&) {}
  virtual void _postvisit(FloatParameter&) {}
  virtual void _postvisit(Assignment&) {}
  virtual void _postvisit(CommaSeparatedStatements&) {}
  virtual void _postvisit(value_t&) {}
  virtual void _postvisit(FuncCall&) {}
  virtual void _postvisit(ExpressionList&) {}
  virtual void _postvisit(Variable&) {}
  virtual void _postvisit(ArrayCall&) {}
  virtual void _postvisit(Dereference&) {}
  virtual void _postvisit(LValueOperator&) {}
  virtual void _postvisit(double&) {}
  virtual void _postvisit(int64_t&) {}
  };

COMPILER_END
