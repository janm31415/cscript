#include "constfold.h"
#include "context.h"
#include "visitor.h"
#include "constant.h"
#include "dump.h"
#include "cscript.h"
#include "func.h"
#include "compiler.h"
#include "vm.h"
#include "map.h"

#include <math.h>
#include <string.h>

typedef struct cscript_constant_folding_visitor
  {
  cscript_visitor* visitor;
  } cscript_constant_folding_visitor;

static cscript_object* find_primitive(cscript_context* ctxt, cscript_string* s)
  {
  cscript_object key;
  key.type = cscript_object_type_string;
  key.value.s = *s;
  cscript_object* res = cscript_map_get(ctxt, ctxt->global->primitives_map, &key);
  return res;
  }

static void postvisit_factor(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_factor* f)
  {
  UNUSED(v);
  if (f->type != cscript_factor_type_number && cscript_is_constant_factor(ctxt, f))
    {
    cscript_vector values = cscript_get_constant_value_factor(ctxt, f);
    if (values.vector_size == 1)
      {
      cscript_factor_destroy(ctxt, f);
      f->type = cscript_factor_type_number;
      f->sign = '+';
      f->factor.number.filename = make_null_string();
      cscript_constant_value val = *cscript_vector_begin(&values, cscript_constant_value);
      f->factor.number.number.fx = val.number.fx;
      f->factor.number.type = val.type;
      }
    cscript_vector_destroy(ctxt, &values);
    }
  if (f->type == cscript_factor_type_function)
    {
    cscript_object* fun = find_primitive(ctxt, &f->factor.fun.name);
    if (fun != NULL)
      {
      int args_constant = 1;
      cscript_parsed_expression* it = cscript_vector_begin(&f->factor.fun.args, cscript_parsed_expression);
      cscript_parsed_expression* it_end = cscript_vector_end(&f->factor.fun.args, cscript_parsed_expression);
      for (; it != it_end; ++it)
        {
        if (cscript_is_constant_expression(ctxt, it) == 0)
          {
          args_constant = 0;
          break;
          }
        }
      if (args_constant > 0)
        {
        cscript_dump_visitor* dumper = cscript_dump_visitor_new(ctxt);
        cscript_visit_factor(ctxt, dumper->visitor, f);

        cscript_context* tmp_ctxt = cscript_context_init(ctxt, 256);
        cscript_vector tokens = cscript_script2tokens(tmp_ctxt, dumper->s.string_ptr);
        cscript_program prog = make_program(tmp_ctxt, &tokens);
        cscript_function* compiled_program = cscript_compile_program(tmp_ctxt, &prog);
        cscript_flonum* res = cast(cscript_flonum*, cscript_run(tmp_ctxt, compiled_program));
        cscript_factor_destroy(ctxt, f);
        f->type = cscript_factor_type_number;
        f->sign = '+';
        f->factor.number.filename = make_null_string();
        f->factor.number.number.fl = *res;
        f->factor.number.type = cscript_number_type_flonum;
        cscript_function_free(tmp_ctxt, compiled_program);
        destroy_tokens_vector(tmp_ctxt, &tokens);
        cscript_program_destroy(tmp_ctxt, &prog);
        cscript_context_destroy(tmp_ctxt);

        cscript_dump_visitor_free(ctxt, dumper);
        }
      }
    }
  }

static int compute_constant_value(cscript_constant_value* output, cscript_vector* values1, cscript_vector* values2, int op)
  {
  if (values1->vector_size == 1 && values2->vector_size == 1)
    {
    cscript_constant_value val1 = *cscript_vector_begin(values1, cscript_constant_value);
    cscript_constant_value val2 = *cscript_vector_begin(values2, cscript_constant_value);
    if (val1.type == cscript_number_type_fixnum && val2.type == cscript_number_type_fixnum)
      {
      output->type = cscript_number_type_fixnum;
      switch (op)
        {
        case cscript_op_mul: output->number.fx = val1.number.fx * val2.number.fx; break;
        case cscript_op_div:output->number.fx = val1.number.fx / val2.number.fx; break;
        case cscript_op_percent:output->number.fx = val1.number.fx % val2.number.fx; break;
        case cscript_op_plus:output->number.fx = val1.number.fx + val2.number.fx; break;
        case cscript_op_minus:output->number.fx = val1.number.fx - val2.number.fx; break;
        case cscript_op_less:output->number.fx = val1.number.fx < val2.number.fx ? 1 : 0; break;
        case cscript_op_leq:output->number.fx = val1.number.fx <= val2.number.fx ? 1 : 0; break;
        case cscript_op_greater:output->number.fx = val1.number.fx > val2.number.fx ? 1 : 0; break;
        case cscript_op_geq:output->number.fx = val1.number.fx >= val2.number.fx ? 1 : 0; break;
        case cscript_op_equal:output->number.fx = val1.number.fx == val2.number.fx ? 1 : 0; break;
        case cscript_op_not_equal:  output->number.fx = val1.number.fx != val2.number.fx ? 1 : 0; break;
        }
      }
    else
      {
      cscript_flonum fl1 = val1.type == cscript_number_type_fixnum ? (cscript_flonum)val1.number.fx : val1.number.fl;
      cscript_flonum fl2 = val2.type == cscript_number_type_fixnum ? (cscript_flonum)val2.number.fx : val2.number.fl;
      output->type = cscript_number_type_flonum;
      switch (op)
        {
        case cscript_op_mul: output->number.fl = fl1 * fl2; break;
        case cscript_op_div:output->number.fl = fl1 / fl2; break;
        case cscript_op_percent:output->number.fl = fmod(fl1, fl2); break;
        case cscript_op_plus:output->number.fl = fl1 + fl2; break;
        case cscript_op_minus:output->number.fl = fl1 - fl2; break;
        case cscript_op_less:output->type = cscript_number_type_fixnum; output->number.fx = fl1 < fl2 ? 1 : 0; break;
        case cscript_op_leq:output->type = cscript_number_type_fixnum; output->number.fx = fl1 <= fl2 ? 1 : 0; break;
        case cscript_op_greater:output->type = cscript_number_type_fixnum; output->number.fx = fl1 > fl2 ? 1 : 0; break;
        case cscript_op_geq:output->type = cscript_number_type_fixnum; output->number.fx = fl1 >= fl2 ? 1 : 0; break;
        case cscript_op_equal:output->type = cscript_number_type_fixnum; output->number.fx = fl1 == fl2 ? 1 : 0; break;
        case cscript_op_not_equal:output->type = cscript_number_type_fixnum; output->number.fx = fl1 != fl2 ? 1 : 0; break;
        }
      }
    return 1;
    }
  return 0;
  }

static void postvisit_term(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_term* s)
  {
  UNUSED(v);
  for (int i = 0; i < (int)s->operands.vector_size - 1; ++i)
    {
    cscript_parsed_factor* f1 = cscript_vector_at(&s->operands, i, cscript_parsed_factor);
    cscript_parsed_factor* f2 = cscript_vector_at(&s->operands, i + 1, cscript_parsed_factor);
    if (cscript_is_constant_factor(ctxt, f1) && cscript_is_constant_factor(ctxt, f2))
      {
      cscript_vector values1 = cscript_get_constant_value_factor(ctxt, f1);
      cscript_vector values2 = cscript_get_constant_value_factor(ctxt, f2);
      int* op = cscript_vector_at(&s->fops, i, int);
      cscript_constant_value result;
      if (compute_constant_value(&result, &values1, &values2, *op))
        {
        cscript_factor_destroy(ctxt, f2);
        cscript_vector_erase(&s->operands, &f2, cscript_parsed_factor);
        cscript_vector_erase(&s->fops, &op, int);
        f1 = cscript_vector_at(&s->operands, i, cscript_parsed_factor);
        cscript_factor_destroy(ctxt, f1);
        f1->type = cscript_factor_type_number;
        f1->sign = '+';
        f1->factor.number.filename = make_null_string();
        f1->factor.number.number.fx = result.number.fx;
        f1->factor.number.type = result.type;
        --i;
        }
      cscript_vector_destroy(ctxt, &values1);
      cscript_vector_destroy(ctxt, &values2);
      }
    else
      break;
    }
  }

static void postvisit_relop(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_relop* s)
  {
  UNUSED(v);
  for (int i = 0; i < (int)s->operands.vector_size - 1; ++i)
    {
    cscript_parsed_term* t1 = cscript_vector_at(&s->operands, i, cscript_parsed_term);
    cscript_parsed_term* t2 = cscript_vector_at(&s->operands, i + 1, cscript_parsed_term);
    if (cscript_is_constant_term(ctxt, t1) && cscript_is_constant_term(ctxt, t2))
      {
      cscript_vector values1 = cscript_get_constant_value_term(ctxt, t1);
      cscript_vector values2 = cscript_get_constant_value_term(ctxt, t2);
      int* op = cscript_vector_at(&s->fops, i, int);
      cscript_constant_value result;
      if (compute_constant_value(&result, &values1, &values2, *op))
        {
        cscript_term_destroy(ctxt, t2);
        cscript_vector_erase(&s->operands, &t2, cscript_parsed_term);
        cscript_vector_erase(&s->fops, &op, int);
        t1 = cscript_vector_at(&s->operands, i, cscript_parsed_term);
        cscript_term_destroy(ctxt, t1);
        t1->filename = make_null_string();
        cscript_vector_init(ctxt, &t1->operands, cscript_parsed_factor);
        cscript_vector_init(ctxt, &t1->fops, int);
        cscript_parsed_factor f;
        f.type = cscript_factor_type_number;
        f.sign = '+';
        f.factor.number.filename = make_null_string();
        f.factor.number.number.fx = result.number.fx;
        f.factor.number.type = result.type;
        cscript_vector_push_back(ctxt, &t1->operands, f, cscript_parsed_factor);
        --i;
        }
      cscript_vector_destroy(ctxt, &values1);
      cscript_vector_destroy(ctxt, &values2);
      }
    else
      break;
    }
  }

static void postvisit_expression(cscript_context* ctxt, cscript_visitor* v, cscript_parsed_expression* s)
  {
  UNUSED(v);
  for (int i = 0; i < (int)s->operands.vector_size - 1; ++i)
    {
    cscript_parsed_relop* r1 = cscript_vector_at(&s->operands, i, cscript_parsed_relop);
    cscript_parsed_relop* r2 = cscript_vector_at(&s->operands, i + 1, cscript_parsed_relop);
    if (cscript_is_constant_relop(ctxt, r1) && cscript_is_constant_relop(ctxt, r2))
      {
      cscript_vector values1 = cscript_get_constant_value_relop(ctxt, r1);
      cscript_vector values2 = cscript_get_constant_value_relop(ctxt, r2);
      int* op = cscript_vector_at(&s->fops, i, int);
      cscript_constant_value result;
      if (compute_constant_value(&result, &values1, &values2, *op))
        {
        cscript_relop_destroy(ctxt, r2);
        cscript_vector_erase(&s->operands, &r2, cscript_parsed_relop);
        cscript_vector_erase(&s->fops, &op, int);
        r1 = cscript_vector_at(&s->operands, i, cscript_parsed_relop);
        cscript_relop_destroy(ctxt, r1);
        r1->filename = make_null_string();
        cscript_vector_init(ctxt, &r1->operands, cscript_parsed_term);
        cscript_vector_init(ctxt, &r1->fops, int);
        cscript_parsed_term t;
        t.filename = make_null_string();
        cscript_vector_init(ctxt, &t.operands, cscript_parsed_factor);
        cscript_vector_init(ctxt, &t.fops, int);
        cscript_parsed_factor f;
        f.type = cscript_factor_type_number;
        f.sign = '+';
        f.factor.number.filename = make_null_string();
        f.factor.number.number.fx = result.number.fx;
        f.factor.number.type = result.type;
        cscript_vector_push_back(ctxt, &t.operands, f, cscript_parsed_factor);
        cscript_vector_push_back(ctxt, &r1->operands, t, cscript_parsed_term);
        --i;
        }
      cscript_vector_destroy(ctxt, &values1);
      cscript_vector_destroy(ctxt, &values2);
      }
    else
      break;
    }
  }

static void postvisit_statement(cscript_context* ctxt, cscript_visitor* v, cscript_statement* s)
  {
  if (s->type == cscript_statement_type_if)
    {
    cscript_parsed_expression* cond = cscript_vector_begin(&s->statement.iftest.condition, cscript_parsed_expression);
    if (cscript_is_constant_expression(ctxt, cond))
      {
      cscript_vector res = cscript_get_constant_value_expression(ctxt, cond);
      if (res.vector_size == 1)
        {
        cscript_constant_value val = *cscript_vector_begin(&res, cscript_constant_value);
        cscript_statement stmt;
        if (val.number.fx == 0)
          {
          if (s->statement.iftest.alternative.vector_size > 0)
            {
            stmt = *cscript_vector_begin(&s->statement.iftest.alternative, cscript_statement);
            s->statement.iftest.alternative.vector_size = 0;
            }
          else
            {
            stmt.type = cscript_statement_type_nop;
            stmt.statement.nop.filename = make_null_string();
            }
          }
        else
          {
          stmt = *cscript_vector_begin(&s->statement.iftest.body, cscript_statement);
          s->statement.iftest.body.vector_size = 0;
          }
        cscript_statement_destroy(ctxt, s);
        *s = stmt;
        }
      cscript_vector_destroy(ctxt, &res);
      }
    }
  }

static cscript_constant_folding_visitor* cscript_constant_folding_visitor_new(cscript_context* ctxt)
  {
  cscript_constant_folding_visitor* v = cscript_new(ctxt, cscript_constant_folding_visitor);
  v->visitor = cscript_visitor_new(ctxt, v);
  v->visitor->postvisit_expression = postvisit_expression;
  v->visitor->postvisit_relop = postvisit_relop;
  v->visitor->postvisit_term = postvisit_term;
  v->visitor->postvisit_factor = postvisit_factor;
  v->visitor->postvisit_statement = postvisit_statement;
  return v;
  }

static void cscript_constant_folding_visitor_free(cscript_context* ctxt, cscript_constant_folding_visitor* v)
  {
  if (v)
    {
    v->visitor->destroy(ctxt, v->visitor);
    cscript_delete(ctxt, v);
    }
  }

void cscript_constant_folding(cscript_context* ctxt, cscript_program* program)
  {
  cscript_constant_folding_visitor* v = cscript_constant_folding_visitor_new(ctxt);
  cscript_visit_program(ctxt, v->visitor, program);
  cscript_constant_folding_visitor_free(ctxt, v);
  }