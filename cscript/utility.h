#pragma once

#include "namespace.h"

#include "cscript_api.h"
#include "parse.h"

COMPILER_BEGIN

COMPILER_CSCRIPT_API bool is_empty(const Expression& expr);


double d(value_t v);
int64_t i(value_t v);
bool is_d(value_t v);
bool is_i(value_t v);

double to_d(value_t v);
int64_t to_i(value_t v);
bool is_constant(const Factor& f);
bool is_constant(const Term& t);
bool is_constant(const Relop& r);
bool is_constant(const Expression& expr);
bool is_constant(const std::vector<Expression>& exprs);

value_t get_constant_value(const Factor& f);
value_t get_constant_value(const Term& t);
value_t get_constant_value(const Relop& r);
value_t get_constant_value(const Expression& expr);

void set_constant_value(Factor& f, value_t v);
void set_constant_value(Term& t, value_t v);
void set_constant_value(Relop& r, value_t v);
void set_constant_value(Expression& expr, value_t v);

bool equals(const Variable& v, const Term& t);
bool equals(const Variable& v, const Factor& f);
bool equals(const ArrayCall& v, const Term& t);
bool equals(const ArrayCall& v, const Factor& f);
bool equals(const Dereference& v, const Term& t);
bool equals(const Dereference& v, const Factor& f);

bool equals(const Expression& left, const Expression& right);
bool equals(const Relop& left, const Relop& right);
bool equals(const Term& left, const Term& right);
bool equals(const Factor& left, const Factor& right);
bool equals(const value_t& left, const value_t& right);

bool is_one(const Expression& expr);
bool is_one(const Relop& expr);
bool is_one(const Term& expr);
bool is_one(const Factor& expr);
bool is_one(const value_t& expr);

bool is_minus_one(const Expression& expr);
bool is_minus_one(const Relop& expr);
bool is_minus_one(const Term& expr);
bool is_minus_one(const Factor& expr);
bool is_minus_one(const value_t& expr);

bool is_zero(const Expression& expr);
bool is_zero(const Relop& expr);
bool is_zero(const Term& expr);
bool is_zero(const Factor& expr);
bool is_zero(const value_t& expr);

bool is_simple_relative_operation(const Statement& stm);

COMPILER_END