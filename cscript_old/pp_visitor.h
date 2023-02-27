#pragma once

#include "namespace.h"

#include "visitor.h"

#include <iostream>

COMPILER_BEGIN

struct pretty_print_visitor : public base_visitor<pretty_print_visitor>
  {
  int indent;
  std::ostream* str;

  pretty_print_visitor() : indent(0), str(&std::cout) {}
  virtual ~pretty_print_visitor() {}
  void print_indent()
    {
    for (int i = 0; i < indent; ++i)
      *str << " ";
    }

  void inc()
    {
    indent += 2;
    }

  void dec()
    {
    indent -= 2;
    }

  virtual void _previsit(Nop&) { print_indent(); *str << "NOP\n"; }
  virtual void _previsit(Expression& e)
    {
    print_indent(); *str << "EXPR ";
    for (auto op : e.fops)
      *str << op << " ";
    *str << "\n";
    inc();
    }
  virtual void _postvisit(Expression&)
    {
    dec();
    }
  virtual void _previsit(Relop& e)
    {
    print_indent(); *str << "RELOP ";
    for (auto op : e.fops)
      *str << op << " ";
    *str << "\n";
    inc();
    }
  virtual void _postvisit(Relop&)
    {
    dec();
    }
  virtual void _previsit(Term& e)
    {
    print_indent(); *str << "TERM ";
    for (auto op : e.fops)
      *str << op << " ";
    *str << "\n";
    inc();
    }
  virtual void _postvisit(Term&)
    {
    dec();
    }
  virtual void _previsit(Factor& e)
    {
    print_indent(); *str << "FACTOR";
    if (e.sign == '-')
      *str << " negative";
    *str << "\n";
    inc();
    }
  virtual void _postvisit(Factor&)
    {
    dec();
    }
  virtual void _previsit(LValue&)
    {
    print_indent(); *str << "LVALUE\n";
    inc();
    }
  virtual void _postvisit(LValue&)
    {
    dec();
    }
  virtual void _previsit(For&)
    {
    print_indent(); *str << "FOR\n"; inc();
    }
  virtual void _postvisit(For&)
    {
    dec();
    }
  virtual void _previsit(Int& i)
    {
    print_indent(); *str << "INT " << i.name << "\n";
    inc();
    }
  virtual void _postvisit(Int&)
    {
    dec();
    }
  virtual void _previsit(Float& f)
    {
    print_indent(); *str << "FLOAT " << f.name << "\n";
    inc();
    }
  virtual void _postvisit(Float&)
    {
    dec();
    }
  virtual void _previsit(Assignment& a)
    {
    print_indent();
    *str << "ASSIGN " << a.name << " " << a.op << "\n";
    inc();
    }
  virtual void _postvisit(Assignment&)
    {
    dec();
    }
  virtual void _previsit(value_t&) {}
  virtual void _previsit(FuncCall& f)
    {
    print_indent();
    *str << "FUNCCALL " << f.name << "\n";
    inc();
    }
  virtual void _postvisit(FuncCall&)
    {
    dec();
    }
  virtual void _previsit(Variable& v)
    {
    print_indent();
    *str << "VAR " << v.name << "\n";
    }
  virtual void _previsit(ArrayCall& a)
    {
    print_indent();
    *str << "ARRAYCALL " << a.name << "\n";
    inc();
    }
  virtual void _postvisit(ArrayCall&)
    {
    dec();
    }
  virtual void _previsit(Dereference& d)
    {
    print_indent();
    *str << "*" << d.name << "\n";
    }
  virtual void _previsit(LValueOperator& lv)
    {
    print_indent();
    *str << "LVALUEOPERATOR " << lv.name << "\n";
    inc();
    }
  virtual void _postvisit(LValueOperator&)
    {
    dec();
    }
  virtual void _previsit(double& d) { print_indent(); *str << d << "\n"; }
  virtual void _previsit(int64_t& i) { print_indent(); *str << i << "\n"; }
  };

COMPILER_END