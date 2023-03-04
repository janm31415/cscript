#include "compiler.h"
#include "vector.h"
#include "error.h"
#include "vm.h"
#include "object.h"
#include "primitives.h"
#include "context.h"
#include "environment.h"
#include "constant.h"

#include <string.h>

static void make_code_abx(cscript_context* ctxt, cscript_function* fun, cscript_opcode opc, int a, int bx)
  {
  cscript_instruction i = 0;
  CSCRIPT_SET_OPCODE(i, opc);
  CSCRIPT_SETARG_A(i, a);
  CSCRIPT_SETARG_Bx(i, bx);
  cscript_vector_push_back(ctxt, &fun->code, i, cscript_instruction);
  }

static void make_code_asbx(cscript_context* ctxt, cscript_function* fun, cscript_opcode opc, int a, int sbx)
  {
  cscript_instruction i = 0;
  CSCRIPT_SET_OPCODE(i, opc);
  CSCRIPT_SETARG_A(i, a);
  CSCRIPT_SETARG_sBx(i, sbx);
  cscript_vector_push_back(ctxt, &fun->code, i, cscript_instruction);
  }

static void make_code_ab(cscript_context* ctxt, cscript_function* fun, cscript_opcode opc, int a, int b)
  {
  cscript_instruction i = 0;
  CSCRIPT_SET_OPCODE(i, opc);
  CSCRIPT_SETARG_A(i, a);
  CSCRIPT_SETARG_B(i, b);
  cscript_vector_push_back(ctxt, &fun->code, i, cscript_instruction);
  }

static void make_code_abc(cscript_context* ctxt, cscript_function* fun, cscript_opcode opc, int a, int b, int c)
  {
  cscript_instruction i = 0;
  CSCRIPT_SET_OPCODE(i, opc);
  CSCRIPT_SETARG_A(i, a);
  CSCRIPT_SETARG_B(i, b);
  CSCRIPT_SETARG_C(i, c);
  cscript_vector_push_back(ctxt, &fun->code, i, cscript_instruction);
  }

static int get_k(cscript_context* ctxt, cscript_function* fun, cscript_object* k)
  {
  const cscript_object* idx = cscript_map_get(ctxt, fun->constants_map, k);
  if (idx != NULL && cscript_object_get_type(idx) == cscript_object_type_fixnum)
    {
    cscript_object_destroy(ctxt, k); // the key should be destroyed as the object was already added to the constants map
    return cast(int, idx->value.fx);
    }
  else
    {
    cscript_object* new_id = cscript_map_insert(ctxt, fun->constants_map, k);
    new_id->type = cscript_object_type_fixnum;
    new_id->value.fx = fun->constants.vector_size;
    cscript_vector_push_back(ctxt, &fun->constants, k->value.fx, cscript_fixnum);
    return cast(int, new_id->value.fx);
    }
  }

#define cscript_reg_typeinfo_fixnum 0
#define cscript_reg_typeinfo_flonum 1
#define cscript_reg_typeinfo_fixnum_pointer 2
#define cscript_reg_typeinfo_flonum_pointer 3

typedef struct compiler_state
  {
  int freereg;
  int reg_typeinfo;
  cscript_function* fun;
  } compiler_state;

compiler_state init_compiler_state(int freereg, int typeinfo, cscript_function* fun)
  {
  compiler_state state;
  state.freereg = freereg;
  state.reg_typeinfo = typeinfo;
  state.fun = fun;
  return state;
  }

static void compile_number(cscript_context* ctxt, compiler_state* state, cscript_parsed_number* n)
  {
  cscript_object obj;
  obj.type = cscript_object_type_undefined;
  switch (n->type)
    {
    case cscript_number_type_fixnum:
    {
    state->reg_typeinfo = cscript_reg_typeinfo_fixnum;
    cscript_fixnum fx = n->number.fx;
    if (fx <= CSCRIPT_MAXARG_sBx && fx >= -CSCRIPT_MAXARG_sBx)
      {
      make_code_asbx(ctxt, state->fun, CSCRIPT_OPCODE_SETFIXNUM, state->freereg, cast(int, fx));
      return;
      }
    obj = make_cscript_object_fixnum(fx);
    break;
    }
    case cscript_number_type_flonum:
    {
    state->reg_typeinfo = cscript_reg_typeinfo_flonum;
    cscript_flonum fl = n->number.fl;
    obj = make_cscript_object_flonum(fl);
    break;
    }
    }
  int k_pos = get_k(ctxt, state->fun, &obj);
  make_code_abx(ctxt, state->fun, CSCRIPT_OPCODE_LOADK, state->freereg, k_pos);
  }

static void compile_expression(cscript_context* ctxt, compiler_state* state, cscript_parsed_expression* e);
static void compile_statement(cscript_context* ctxt, compiler_state* state, cscript_statement* stmt);

static void compile_global_variable(cscript_context* ctxt, compiler_state* state, cscript_parsed_variable* v)
  {
  cscript_assert(0);
  }

static void compile_local_variable(cscript_context* ctxt, compiler_state* state, cscript_parsed_variable* v)
  {
  cscript_environment_entry entry;
  if (!cscript_environment_find_recursive(&entry, ctxt, &v->name))
    {
    cscript_compile_error_cstr(ctxt, CSCRIPT_ERROR_VARIABLE_UNKNOWN, v->line_nr, v->column_nr, &v->filename, v->name.string_ptr);
    }
  else
    {
    if (v->dims.vector_size > 0)
      {
      if (entry.register_type > 1) // pointer type
        {
        cscript_assert(v->dereference == 0); // dereference is todo
        compile_expression(ctxt, state, cscript_vector_begin(&v->dims, cscript_parsed_expression));
        if (state->reg_typeinfo == cscript_reg_typeinfo_flonum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg, cscript_number_type_fixnum);
          state->reg_typeinfo = cscript_number_type_fixnum;
          }
        /*
        freereg + 0: dim*8
        freereg + 1: 8
        */
        make_code_asbx(ctxt, state->fun, CSCRIPT_OPCODE_SETFIXNUM, state->freereg + 1, 8);
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, CSCRIPT_MUL_FIXNUM);
        /*
        freereg + 0: dim*8
        freereg + 1: address
        */
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg + 1, (int)entry.position);
        /*
        freereg + 0: dim*8 + address
        freereg + 1: address
        */
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, CSCRIPT_ADD_FIXNUM);
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_LOAD_MEMORY, state->freereg, state->freereg);
        state->reg_typeinfo = entry.register_type & 1;
        }
      else
        {
        cscript_assert(v->dereference == 0); // dereference is todo
        compile_expression(ctxt, state, cscript_vector_begin(&v->dims, cscript_parsed_expression));
        if (state->reg_typeinfo == cscript_reg_typeinfo_flonum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg, cscript_number_type_fixnum);
          state->reg_typeinfo = cscript_number_type_fixnum;
          }
        make_code_abc(ctxt, state->fun, CSCRIPT_OPCODE_MOVE_FROM_ARR, state->freereg, (int)entry.position, state->freereg);
        state->reg_typeinfo = entry.register_type;
        }
      }
    else
      {
      if (v->dereference != 0)
        {
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_LOAD_MEMORY, state->freereg, (int)entry.position);
        }
      else
        {
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg, (int)entry.position);
        }
      state->reg_typeinfo = entry.register_type & 1;
      }
    }
  }

static void compile_variable(cscript_context* ctxt, compiler_state* state, cscript_parsed_variable* v)
  {
  if (v->name.string_ptr[0] == '$')
    compile_global_variable(ctxt, state, v);
  else
    compile_local_variable(ctxt, state, v);
  }

static void compile_lvalue_operator(cscript_context* ctxt, compiler_state* state, cscript_parsed_lvalue_operator* lvop)
  {
  int adder = 1;
  if (strcmp(lvop->name.string_ptr, "--") == 0)
    adder = -1;
  cscript_environment_entry entry;
  if (!cscript_environment_find_recursive(&entry, ctxt, &lvop->lvalue.name))
    {
    cscript_compile_error_cstr(ctxt, CSCRIPT_ERROR_VARIABLE_UNKNOWN, lvop->lvalue.line_nr, lvop->lvalue.column_nr, &lvop->lvalue.filename, lvop->lvalue.name.string_ptr);
    }
  else
    {
    if (lvop->lvalue.dims.vector_size > 0)
      {
      if (entry.register_type > 1) // pointer type
        {
        cscript_assert(lvop->lvalue.dereference == 0); // dereference is todo
        compile_expression(ctxt, state, cscript_vector_begin(&lvop->lvalue.dims, cscript_parsed_expression));
        if (state->reg_typeinfo == cscript_reg_typeinfo_flonum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg, cscript_number_type_fixnum);
          state->reg_typeinfo = cscript_number_type_fixnum;
          }
        /*
        freereg + 0: dim*8
        freereg + 1: 8
        */
        make_code_asbx(ctxt, state->fun, CSCRIPT_OPCODE_SETFIXNUM, state->freereg + 1, 8);
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, CSCRIPT_MUL_FIXNUM);
        /*
        freereg + 0: dim*8
        freereg + 1: address
        */
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg + 1, (int)entry.position);
        /*
        freereg + 0: dim*8 + address
        freereg + 1: address
        */
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, CSCRIPT_ADD_FIXNUM);
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_LOAD_MEMORY, state->freereg + 1, state->freereg);
        make_code_asbx(ctxt, state->fun, CSCRIPT_OPCODE_SETFIXNUM, state->freereg + 2, adder);
        if ((entry.register_type & 1) == cscript_reg_typeinfo_flonum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg + 2, cscript_number_type_flonum);
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg + 1, CSCRIPT_ADD_FLONUM);
          }
        else
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg + 1, CSCRIPT_ADD_FIXNUM);
          }
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_STORE_MEMORY, state->freereg, state->freereg + 1);
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg, state->freereg + 1);
        state->reg_typeinfo = entry.register_type & 1;
        }
      else
        {
        cscript_assert(lvop->lvalue.dereference == 0); // dereference is todo
        compile_expression(ctxt, state, cscript_vector_begin(&lvop->lvalue.dims, cscript_parsed_expression));
        if (state->reg_typeinfo == cscript_reg_typeinfo_flonum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg, cscript_number_type_fixnum);
          state->reg_typeinfo = cscript_number_type_fixnum;
          }
        make_code_abc(ctxt, state->fun, CSCRIPT_OPCODE_MOVE_FROM_ARR, state->freereg + 1, (int)entry.position, state->freereg);
        make_code_asbx(ctxt, state->fun, CSCRIPT_OPCODE_SETFIXNUM, state->freereg + 2, adder);
        if ((entry.register_type & 1) == cscript_reg_typeinfo_flonum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg + 2, cscript_number_type_flonum);
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg + 1, CSCRIPT_ADD_FLONUM);
          }
        else
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg + 1, CSCRIPT_ADD_FIXNUM);
          }
        make_code_abc(ctxt, state->fun, CSCRIPT_OPCODE_MOVE_TO_ARR, (int)entry.position, state->freereg, state->freereg + 1);
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg, state->freereg + 1);
        state->reg_typeinfo = entry.register_type & 1;
        }
      }
    else
      {
      if (lvop->lvalue.dereference != 0)
        {
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_LOAD_MEMORY, state->freereg, (int)entry.position);
        make_code_asbx(ctxt, state->fun, CSCRIPT_OPCODE_SETFIXNUM, state->freereg + 1, adder);
        if ((entry.register_type & 1) == cscript_reg_typeinfo_flonum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg + 1, cscript_number_type_flonum);
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, CSCRIPT_ADD_FLONUM);
          }
        else
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, CSCRIPT_ADD_FIXNUM);
          }
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_STORE_MEMORY, (int)entry.position, state->freereg);
        }
      else
        {
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg, (int)entry.position);
        make_code_asbx(ctxt, state->fun, CSCRIPT_OPCODE_SETFIXNUM, state->freereg + 1, adder);
        if ((entry.register_type & 1) == cscript_reg_typeinfo_flonum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg + 1, cscript_number_type_flonum);
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, CSCRIPT_ADD_FLONUM);
          }
        else
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, CSCRIPT_ADD_FIXNUM);
          }
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, (int)entry.position, state->freereg);
        }
      state->reg_typeinfo = entry.register_type & 1;
      }
    }
  }

static cscript_object* find_primitive(cscript_context* ctxt, cscript_string* s)
  {
  cscript_object key;
  key.type = cscript_object_type_string;
  key.value.s = *s;
  cscript_object* res = cscript_map_get(ctxt, ctxt->global->primitives_map, &key);
  return res;
  }

static void compile_function(cscript_context* ctxt, compiler_state* state, cscript_parsed_function* f)
  {
  cscript_object* fun = find_primitive(ctxt, &f->name);
  if (fun == NULL)
    {
    cscript_compile_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, f->line_nr, f->column_nr, &f->filename, "function unknown");
    return;
    }
  else
    {
    int freereg = state->freereg;
    cscript_parsed_expression* it = cscript_vector_begin(&f->args, cscript_parsed_expression);
    cscript_parsed_expression* it_end = cscript_vector_end(&f->args, cscript_parsed_expression);
    for (; it != it_end; ++it)
      {
      compile_expression(ctxt, state, it);
      if (state->reg_typeinfo != cscript_reg_typeinfo_flonum)
        {
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg, cscript_number_type_flonum);
        }
      ++state->freereg;
      }
    state->freereg = freereg;
    state->reg_typeinfo = cscript_reg_typeinfo_flonum;
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, fun->value.fx);
    }
  }

static void compile_factor(cscript_context* ctxt, compiler_state* state, cscript_parsed_factor* f)
  {
  switch (f->type)
    {
    case cscript_factor_type_number:
      compile_number(ctxt, state, &f->factor.number);
      break;
    case cscript_factor_type_expression:
      compile_expression(ctxt, state, &f->factor.expr);
      break;
    case cscript_factor_type_variable:
      compile_variable(ctxt, state, &f->factor.var);
      break;
    case cscript_factor_type_lvalue_operator:
      compile_lvalue_operator(ctxt, state, &f->factor.lvop);
      break;
    case cscript_factor_type_function:
      compile_function(ctxt, state, &f->factor.fun);
      break;
    default:
      cscript_throw(ctxt, CSCRIPT_ERROR_NOT_IMPLEMENTED);
    }
  if (f->sign == '-')
    {
    if (state->reg_typeinfo == cscript_reg_typeinfo_fixnum)
      {
      make_code_asbx(ctxt, state->fun, CSCRIPT_OPCODE_SETFIXNUM, state->freereg + 1, -1);
      make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, CSCRIPT_MUL_FIXNUM);
      }
    else
      {
      make_code_asbx(ctxt, state->fun, CSCRIPT_OPCODE_SETFIXNUM, state->freereg + 1, -1);
      make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg + 1, cscript_number_type_flonum);
      make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, CSCRIPT_MUL_FLONUM);
      }
    }
  }

static void compile_term(cscript_context* ctxt, compiler_state* state, cscript_parsed_term* e)
  {
  int freereg = state->freereg;
  cscript_parsed_factor* it = cscript_vector_begin(&e->operands, cscript_parsed_factor);
  cscript_parsed_factor* it_end = cscript_vector_end(&e->operands, cscript_parsed_factor);
  int* op_it = cscript_vector_begin(&e->fops, int);
  //int* op_it_end = cscript_vector_end(&e->fops, int);
  compile_factor(ctxt, state, it);
  int reg_a_typeinfo = state->reg_typeinfo;
  ++it;
  while (it != it_end)
    {
    state->freereg = freereg + 1;
    compile_factor(ctxt, state, it);
    int reg_b_typeinfo = state->reg_typeinfo;
    if (reg_a_typeinfo != reg_b_typeinfo)
      {
      if (reg_a_typeinfo != cscript_reg_typeinfo_flonum)
        {
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, freereg, cscript_number_type_flonum);
        reg_a_typeinfo = cscript_reg_typeinfo_flonum;
        }
      if (reg_b_typeinfo != cscript_reg_typeinfo_flonum)
        {
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg, cscript_number_type_flonum);
        reg_b_typeinfo = cscript_reg_typeinfo_flonum;
        }
      }
    switch (*op_it)
      {
      case cscript_op_mul:
        if (reg_a_typeinfo == cscript_reg_typeinfo_fixnum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_MUL_FIXNUM);
          }
        else
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_MUL_FLONUM);
          }
        break;
      case cscript_op_div:
        if (reg_a_typeinfo == cscript_reg_typeinfo_fixnum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_DIV_FIXNUM);
          }
        else
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_DIV_FLONUM);
          }
        break;
      case cscript_op_percent:
        if (reg_a_typeinfo == cscript_reg_typeinfo_fixnum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_MOD_FIXNUM);
          }
        else
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_MOD_FLONUM);
          }
        break;
      default:
        cscript_throw(ctxt, CSCRIPT_ERROR_NOT_IMPLEMENTED);
      }
    ++it;
    ++op_it;
    }
  state->freereg = freereg;
  state->reg_typeinfo = reg_a_typeinfo;
  }

static void compile_relop(cscript_context* ctxt, compiler_state* state, cscript_parsed_relop* e)
  {
  int freereg = state->freereg;
  cscript_parsed_term* it = cscript_vector_begin(&e->operands, cscript_parsed_term);
  cscript_parsed_term* it_end = cscript_vector_end(&e->operands, cscript_parsed_term);
  int* op_it = cscript_vector_begin(&e->fops, int);
  //int* op_it_end = cscript_vector_end(&e->fops, int);
  compile_term(ctxt, state, it);
  int reg_a_typeinfo = state->reg_typeinfo;
  ++it;
  while (it != it_end)
    {
    state->freereg = freereg + 1;
    compile_term(ctxt, state, it);
    int reg_b_typeinfo = state->reg_typeinfo;
    if (reg_a_typeinfo != reg_b_typeinfo)
      {
      if (reg_a_typeinfo != cscript_reg_typeinfo_flonum)
        {
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, freereg, cscript_number_type_flonum);
        reg_a_typeinfo = cscript_reg_typeinfo_flonum;
        }
      if (reg_b_typeinfo != cscript_reg_typeinfo_flonum)
        {
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg, cscript_number_type_flonum);
        reg_b_typeinfo = cscript_reg_typeinfo_flonum;
        }
      }

    switch (*op_it)
      {
      case cscript_op_plus:
        if (reg_a_typeinfo == cscript_reg_typeinfo_fixnum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_ADD_FIXNUM);
          }
        else
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_ADD_FLONUM);
          }
        break;
      case cscript_op_minus:
        if (reg_a_typeinfo == cscript_reg_typeinfo_fixnum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_SUB_FIXNUM);
          }
        else
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_SUB_FLONUM);
          }
        break;
      default:
        cscript_throw(ctxt, CSCRIPT_ERROR_NOT_IMPLEMENTED);
      }
    ++it;
    ++op_it;
    }
  state->freereg = freereg;
  state->reg_typeinfo = reg_a_typeinfo;
  }

static void compile_expression(cscript_context* ctxt, compiler_state* state, cscript_parsed_expression* e)
  {
  int freereg = state->freereg;
  cscript_parsed_relop* it = cscript_vector_begin(&e->operands, cscript_parsed_relop);
  cscript_parsed_relop* it_end = cscript_vector_end(&e->operands, cscript_parsed_relop);
  int* op_it = cscript_vector_begin(&e->fops, int);
  //int* op_it_end = cscript_vector_end(&e->fops, int);
  compile_relop(ctxt, state, it);
  int reg_a_typeinfo = state->reg_typeinfo;
  ++it;
  while (it != it_end)
    {
    state->freereg = freereg + 1;
    compile_relop(ctxt, state, it);
    int reg_b_typeinfo = state->reg_typeinfo;
    if (reg_a_typeinfo != reg_b_typeinfo)
      {
      if (reg_a_typeinfo != cscript_reg_typeinfo_flonum)
        {
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, freereg, cscript_number_type_flonum);
        reg_a_typeinfo = cscript_reg_typeinfo_flonum;
        }
      if (reg_b_typeinfo != cscript_reg_typeinfo_flonum)
        {
        make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg, cscript_number_type_flonum);
        reg_b_typeinfo = cscript_reg_typeinfo_flonum;
        }
      }
    switch (*op_it)
      {
      case cscript_op_less:
        if (reg_a_typeinfo == cscript_reg_typeinfo_fixnum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_LESS_FIXNUM);
          }
        else
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_LESS_FLONUM);
          }
        reg_a_typeinfo = cscript_reg_typeinfo_fixnum;
        break;
      case cscript_op_leq:
        if (reg_a_typeinfo == cscript_reg_typeinfo_fixnum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_LEQ_FIXNUM);
          }
        else
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_LEQ_FLONUM);
          }
        reg_a_typeinfo = cscript_reg_typeinfo_fixnum;
        break;
      case cscript_op_greater:
        if (reg_a_typeinfo == cscript_reg_typeinfo_fixnum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_GREATER_FIXNUM);
          }
        else
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_GREATER_FLONUM);
          }
        reg_a_typeinfo = cscript_reg_typeinfo_fixnum;
        break;
      case cscript_op_geq:
        if (reg_a_typeinfo == cscript_reg_typeinfo_fixnum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_GEQ_FIXNUM);
          }
        else
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_GEQ_FLONUM);
          }
        reg_a_typeinfo = cscript_reg_typeinfo_fixnum;
        break;
      case cscript_op_equal:
        if (reg_a_typeinfo == cscript_reg_typeinfo_fixnum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_EQUAL_FIXNUM);
          }
        else
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_EQUAL_FLONUM);
          }
        reg_a_typeinfo = cscript_reg_typeinfo_fixnum;
        break;
      case cscript_op_not_equal:
        if (reg_a_typeinfo == cscript_reg_typeinfo_fixnum)
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_NOT_EQUAL_FIXNUM);
          }
        else
          {
          make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, freereg, CSCRIPT_NOT_EQUAL_FLONUM);
          }
        reg_a_typeinfo = cscript_reg_typeinfo_fixnum;
        break;
      default:
        cscript_throw(ctxt, CSCRIPT_ERROR_NOT_IMPLEMENTED);
      }
    ++it;
    ++op_it;
    }
  state->freereg = freereg;
  state->reg_typeinfo = reg_a_typeinfo;
  }

static void compile_comma_seperated_statements(cscript_context* ctxt, compiler_state* state, cscript_comma_separated_statements* stmts)
  {
  cscript_statement* it = cscript_vector_begin(&stmts->statements, cscript_statement);
  cscript_statement* it_end = cscript_vector_end(&stmts->statements, cscript_statement);
  for (; it != it_end; ++it)
    {
    compile_statement(ctxt, state, it);
    }
  }

static void compile_fixnum_array(cscript_context* ctxt, compiler_state* state, cscript_parsed_fixnum* fx)
  {
  cscript_parsed_expression* dim_expr = cscript_vector_begin(&fx->dims, cscript_parsed_expression);
  if (cscript_is_constant_expression(dim_expr) == 0)
    {
    cscript_compile_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, fx->line_nr, fx->column_nr, &fx->filename, "array dimension should be a constant");
    return;
    }
  int init = fx->expr.operands.vector_size > 0;
  if (init)
    {
    cscript_assert(0);
    }
  cscript_environment_entry entry;
  if (cscript_environment_find(&entry, ctxt, &fx->name))
    {
    cscript_compile_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, fx->line_nr, fx->column_nr, &fx->filename, "Variable already exists");
    }
  else
    {
    entry.type = CSCRIPT_ENV_TYPE_STACK;
    entry.position = state->freereg;
    entry.register_type = cscript_reg_typeinfo_fixnum;
    cscript_string s;
    cscript_string_copy(ctxt, &s, &fx->name);
    cscript_environment_add(ctxt, &s, entry);
    cscript_constant_value dim_size = cscript_get_constant_value_expression(dim_expr);
    if (dim_size.type == cscript_number_type_fixnum)
      state->freereg += (int)dim_size.number.fx;
    else
      state->freereg += (int)dim_size.number.fl;
    }
  }

static void compile_fixnum_global_single(cscript_context* ctxt, compiler_state* state, cscript_parsed_fixnum* fx)
  {
  cscript_assert(0);
  }

static void compile_fixnum_single(cscript_context* ctxt, compiler_state* state, cscript_parsed_fixnum* fx)
  {
  int init = fx->expr.operands.vector_size > 0;
  if (init)
    {
    compile_expression(ctxt, state, &fx->expr);
    if (state->reg_typeinfo == cscript_reg_typeinfo_flonum)
      {
      make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg, cscript_number_type_fixnum);
      state->reg_typeinfo = cscript_number_type_fixnum;
      }
    }
  cscript_environment_entry entry;
  if (cscript_environment_find(&entry, ctxt, &fx->name))
    {
    cscript_compile_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, fx->line_nr, fx->column_nr, &fx->filename, "Variable already exists");
    }
  else
    {
    entry.type = CSCRIPT_ENV_TYPE_STACK;
    entry.position = state->freereg;
    entry.register_type = cscript_reg_typeinfo_fixnum;
    cscript_string s;
    cscript_string_copy(ctxt, &s, &fx->name);
    cscript_environment_add(ctxt, &s, entry);
    ++state->freereg;
    }
  }

static void compile_fixnum(cscript_context* ctxt, compiler_state* state, cscript_parsed_fixnum* fx)
  {
  if (fx->dims.vector_size > 0)
    {
    if (fx->name.string_ptr[0] == '$')
      {
      cscript_compile_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, fx->line_nr, fx->column_nr, &fx->filename, "Global variables can't be arrays");
      }
    else
      {
      compile_fixnum_array(ctxt, state, fx);
      }
    }
  else
    {
    if (fx->name.string_ptr[0] == '$')
      {
      compile_fixnum_global_single(ctxt, state, fx);
      }
    else
      {
      compile_fixnum_single(ctxt, state, fx);
      }
    }
  }


static void compile_flonum_array(cscript_context* ctxt, compiler_state* state, cscript_parsed_flonum* fl)
  {
  cscript_parsed_expression* dim_expr = cscript_vector_begin(&fl->dims, cscript_parsed_expression);
  if (cscript_is_constant_expression(dim_expr) == 0)
    {
    cscript_compile_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, fl->line_nr, fl->column_nr, &fl->filename, "array dimension should be a constant");
    return;
    }
  int init = fl->expr.operands.vector_size > 0;
  if (init)
    {
    cscript_assert(0);
    }
  cscript_environment_entry entry;
  if (cscript_environment_find(&entry, ctxt, &fl->name))
    {
    cscript_compile_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, fl->line_nr, fl->column_nr, &fl->filename, "Variable already exists");
    }
  else
    {
    entry.type = CSCRIPT_ENV_TYPE_STACK;
    entry.position = state->freereg;
    entry.register_type = cscript_reg_typeinfo_flonum;
    cscript_string s;
    cscript_string_copy(ctxt, &s, &fl->name);
    cscript_environment_add(ctxt, &s, entry);
    cscript_constant_value dim_size = cscript_get_constant_value_expression(dim_expr);
    if (dim_size.type == cscript_number_type_fixnum)
      state->freereg += (int)dim_size.number.fx;
    else
      state->freereg += (int)dim_size.number.fl;
    }
  }

static void compile_flonum_global_single(cscript_context* ctxt, compiler_state* state, cscript_parsed_flonum* fx)
  {
  cscript_assert(0);
  }

static void compile_flonum_single(cscript_context* ctxt, compiler_state* state, cscript_parsed_flonum* fl)
  {
  int init = fl->expr.operands.vector_size > 0;
  if (init)
    {
    compile_expression(ctxt, state, &fl->expr);
    if (state->reg_typeinfo == cscript_reg_typeinfo_fixnum)
      {
      make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg, cscript_number_type_flonum);
      state->reg_typeinfo = cscript_number_type_flonum;
      }
    }
  cscript_environment_entry entry;
  if (cscript_environment_find(&entry, ctxt, &fl->name))
    {
    cscript_compile_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, fl->line_nr, fl->column_nr, &fl->filename, "Variable already exists");
    }
  else
    {
    entry.type = CSCRIPT_ENV_TYPE_STACK;
    entry.position = state->freereg;
    entry.register_type = cscript_reg_typeinfo_flonum;
    cscript_string s;
    cscript_string_copy(ctxt, &s, &fl->name);
    cscript_environment_add(ctxt, &s, entry);
    ++state->freereg;
    }
  }

static void compile_flonum(cscript_context* ctxt, compiler_state* state, cscript_parsed_flonum* fx)
  {
  if (fx->dims.vector_size > 0)
    {
    if (fx->name.string_ptr[0] == '$')
      {
      cscript_compile_error_cstr(ctxt, CSCRIPT_ERROR_BAD_SYNTAX, fx->line_nr, fx->column_nr, &fx->filename, "Global variables can't be arrays");
      }
    else
      {
      compile_flonum_array(ctxt, state, fx);
      }
    }
  else
    {
    if (fx->name.string_ptr[0] == '$')
      {
      compile_flonum_global_single(ctxt, state, fx);
      }
    else
      {
      compile_flonum_single(ctxt, state, fx);
      }
    }
  }

static void compile_assigment_pointer(cscript_context* ctxt, compiler_state* state, cscript_parsed_assignment* a, cscript_environment_entry entry)
  {
  /*
  freereg + 0: dim
  */
  cscript_parsed_expression* e = cscript_vector_begin(&a->dims, cscript_parsed_expression);
  compile_expression(ctxt, state, e);
  if (state->reg_typeinfo != cscript_reg_typeinfo_fixnum)
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg, cscript_reg_typeinfo_fixnum);
    state->reg_typeinfo = cscript_reg_typeinfo_fixnum;
    }
  /*
  freereg + 0: dim*8
  freereg + 1: 8
  */
  make_code_asbx(ctxt, state->fun, CSCRIPT_OPCODE_SETFIXNUM, state->freereg + 1, 8);
  make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, CSCRIPT_MUL_FIXNUM);
  /*
  freereg + 0: dim*8
  freereg + 1: address
  */
  make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg + 1, (int)entry.position);
  /*
  freereg + 0: dim*8 + address
  freereg + 1: address
  */
  make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, CSCRIPT_ADD_FIXNUM);

  /*
  freereg + 0: dim*8 + address
  freereg + 1: address
  freereg + 2: expr
  */
  state->freereg += 2;
  compile_expression(ctxt, state, &a->expr);

  if (state->reg_typeinfo != (entry.register_type & 1))
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg, (entry.register_type & 1));
    state->reg_typeinfo = (entry.register_type & 1);
    }
  state->freereg -= 2;

  switch (a->op.string_ptr[0])
    {
    case '=':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_STORE_MEMORY, state->freereg, state->freereg + 2);
    break;
    }
    case '+':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_LOAD_MEMORY, state->freereg + 1, state->freereg);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg + 1, (entry.register_type & 1) == cscript_reg_typeinfo_flonum ? CSCRIPT_ADD_FLONUM : CSCRIPT_ADD_FIXNUM);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_STORE_MEMORY, state->freereg, state->freereg + 1);
    break;
    }
    case '-':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_LOAD_MEMORY, state->freereg + 1, state->freereg);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg + 1, (entry.register_type & 1) == cscript_reg_typeinfo_flonum ? CSCRIPT_SUB_FLONUM : CSCRIPT_SUB_FIXNUM);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_STORE_MEMORY, state->freereg, state->freereg + 1);
    break;
    }
    case '*':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_LOAD_MEMORY, state->freereg + 1, state->freereg);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg + 1, (entry.register_type & 1) == cscript_reg_typeinfo_flonum ? CSCRIPT_MUL_FLONUM : CSCRIPT_MUL_FIXNUM);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_STORE_MEMORY, state->freereg, state->freereg + 1);
    break;
    }
    case '/':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_LOAD_MEMORY, state->freereg + 1, state->freereg);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg + 1, (entry.register_type & 1) == cscript_reg_typeinfo_flonum ? CSCRIPT_DIV_FLONUM : CSCRIPT_DIV_FIXNUM);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_STORE_MEMORY, state->freereg, state->freereg + 1);
    break;
    }
    default:
      break;
    }
  }


static void compile_assigment_dereference(cscript_context* ctxt, compiler_state* state, cscript_parsed_assignment* a, cscript_environment_entry entry)
  {
  make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg, (int)entry.position);
  /*
  freereg + 0: address
  freereg + 2: expr
  */
  state->freereg += 2;
  compile_expression(ctxt, state, &a->expr);

  if (state->reg_typeinfo != (entry.register_type & 1))
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg, (entry.register_type & 1));
    state->reg_typeinfo = (entry.register_type & 1);
    }
  state->freereg -= 2;

  switch (a->op.string_ptr[0])
    {
    case '=':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_STORE_MEMORY, state->freereg, state->freereg + 2);
    break;
    }
    case '+':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_LOAD_MEMORY, state->freereg + 1, state->freereg);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg + 1, (entry.register_type & 1) == cscript_reg_typeinfo_flonum ? CSCRIPT_ADD_FLONUM : CSCRIPT_ADD_FIXNUM);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_STORE_MEMORY, state->freereg, state->freereg + 1);
    break;
    }
    case '-':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_LOAD_MEMORY, state->freereg + 1, state->freereg);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg + 1, (entry.register_type & 1) == cscript_reg_typeinfo_flonum ? CSCRIPT_SUB_FLONUM : CSCRIPT_SUB_FIXNUM);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_STORE_MEMORY, state->freereg, state->freereg + 1);
    break;
    }
    case '*':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_LOAD_MEMORY, state->freereg + 1, state->freereg);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg + 1, (entry.register_type & 1) == cscript_reg_typeinfo_flonum ? CSCRIPT_MUL_FLONUM : CSCRIPT_MUL_FIXNUM);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_STORE_MEMORY, state->freereg, state->freereg + 1);
    break;
    }
    case '/':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_LOAD_MEMORY, state->freereg + 1, state->freereg);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg + 1, (entry.register_type & 1) == cscript_reg_typeinfo_flonum ? CSCRIPT_DIV_FLONUM : CSCRIPT_DIV_FIXNUM);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_STORE_MEMORY, state->freereg, state->freereg + 1);
    break;
    }
    default:
      break;
    }
  }

static void compile_assigment_array(cscript_context* ctxt, compiler_state* state, cscript_parsed_assignment* a, cscript_environment_entry entry)
  {
  cscript_parsed_expression* e = cscript_vector_begin(&a->dims, cscript_parsed_expression);
  compile_expression(ctxt, state, e);
  if (state->reg_typeinfo != cscript_reg_typeinfo_fixnum)
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg, cscript_reg_typeinfo_fixnum);
    state->reg_typeinfo = cscript_reg_typeinfo_fixnum;
    }
  state->freereg += 2;
  compile_expression(ctxt, state, &a->expr);
  if (entry.register_type > cscript_reg_typeinfo_flonum)
    {
    cscript_assert(0);
    }
  if (state->reg_typeinfo != entry.register_type)
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg, entry.register_type);
    state->reg_typeinfo = entry.register_type;
    }
  state->freereg -= 2;
  switch (a->op.string_ptr[0])
    {
    case '=':
    {
    make_code_abc(ctxt, state->fun, CSCRIPT_OPCODE_MOVE_TO_ARR, (int)entry.position, state->freereg, state->freereg + 2);
    break;
    }
    case '+':
    {
    make_code_abc(ctxt, state->fun, CSCRIPT_OPCODE_MOVE_FROM_ARR, state->freereg + 1, (int)entry.position, state->freereg);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg + 1, entry.register_type == cscript_reg_typeinfo_flonum ? CSCRIPT_ADD_FLONUM : CSCRIPT_ADD_FIXNUM);
    make_code_abc(ctxt, state->fun, CSCRIPT_OPCODE_MOVE_TO_ARR, (int)entry.position, state->freereg, state->freereg + 1);
    break;
    }
    case '-':
    {
    make_code_abc(ctxt, state->fun, CSCRIPT_OPCODE_MOVE_FROM_ARR, state->freereg + 1, (int)entry.position, state->freereg);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg + 1, entry.register_type == cscript_reg_typeinfo_flonum ? CSCRIPT_SUB_FLONUM : CSCRIPT_SUB_FIXNUM);
    make_code_abc(ctxt, state->fun, CSCRIPT_OPCODE_MOVE_TO_ARR, (int)entry.position, state->freereg, state->freereg + 1);
    break;
    }
    case '*':
    {
    make_code_abc(ctxt, state->fun, CSCRIPT_OPCODE_MOVE_FROM_ARR, state->freereg + 1, (int)entry.position, state->freereg);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg + 1, entry.register_type == cscript_reg_typeinfo_flonum ? CSCRIPT_MUL_FLONUM : CSCRIPT_MUL_FIXNUM);
    make_code_abc(ctxt, state->fun, CSCRIPT_OPCODE_MOVE_TO_ARR, (int)entry.position, state->freereg, state->freereg + 1);
    break;
    }
    case '/':
    {
    make_code_abc(ctxt, state->fun, CSCRIPT_OPCODE_MOVE_FROM_ARR, state->freereg + 1, (int)entry.position, state->freereg);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg + 1, entry.register_type == cscript_reg_typeinfo_flonum ? CSCRIPT_DIV_FLONUM : CSCRIPT_DIV_FIXNUM);
    make_code_abc(ctxt, state->fun, CSCRIPT_OPCODE_MOVE_TO_ARR, (int)entry.position, state->freereg, state->freereg + 1);
    break;
    }
    default:
      break;
    }
  }

static void compile_assigment_single(cscript_context* ctxt, compiler_state* state, cscript_parsed_assignment* a, cscript_environment_entry entry)
  {
  ++state->freereg;
  compile_expression(ctxt, state, &a->expr);
  --state->freereg;
  if (entry.register_type > cscript_reg_typeinfo_flonum)
    {
    cscript_assert(0);
    }
  if (state->reg_typeinfo != entry.register_type)
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg + 1, entry.register_type);
    state->reg_typeinfo = entry.register_type;
    }
  switch (a->op.string_ptr[0])
    {
    case '=':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, (int)entry.position, state->freereg + 1);
    break;
    }
    case '+':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg, (int)entry.position);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, entry.register_type == cscript_reg_typeinfo_flonum ? CSCRIPT_ADD_FLONUM : CSCRIPT_ADD_FIXNUM);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, (int)entry.position, state->freereg);
    break;
    }
    case '*':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg, (int)entry.position);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, entry.register_type == cscript_reg_typeinfo_flonum ? CSCRIPT_MUL_FLONUM : CSCRIPT_MUL_FIXNUM);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, (int)entry.position, state->freereg);
    break;
    }
    case '-':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg, (int)entry.position);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, entry.register_type == cscript_reg_typeinfo_flonum ? CSCRIPT_SUB_FLONUM : CSCRIPT_SUB_FIXNUM);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, (int)entry.position, state->freereg);
    break;
    }
    case '/':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg, (int)entry.position);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, entry.register_type == cscript_reg_typeinfo_flonum ? CSCRIPT_DIV_FLONUM : CSCRIPT_DIV_FIXNUM);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, (int)entry.position, state->freereg);
    break;
    }
    default:
      break;
    }
  }

static void compile_assigment(cscript_context* ctxt, compiler_state* state, cscript_parsed_assignment* a)
  {
  if (a->name.string_ptr[0] == '$')
    {
    }
  else
    {
    cscript_environment_entry entry;
    if (!cscript_environment_find_recursive(&entry, ctxt, &a->name))
      {
      cscript_compile_error_cstr(ctxt, CSCRIPT_ERROR_VARIABLE_UNKNOWN, a->line_nr, a->column_nr, &a->filename, a->name.string_ptr);
      }
    else
      {
      if (a->dims.vector_size > 0)
        {
        if (entry.register_type > cscript_reg_typeinfo_flonum)
          {
          compile_assigment_pointer(ctxt, state, a, entry);
          }
        else
          {
          compile_assigment_array(ctxt, state, a, entry);
          }
        }
      else if (a->derefence != 0)
        {
        compile_assigment_dereference(ctxt, state, a, entry);
        }
      else
        {
        compile_assigment_single(ctxt, state, a, entry);
        }
      }
    }
  }

static void compile_for(cscript_context* ctxt, compiler_state* state, cscript_parsed_for* f)
  {
  cscript_environment_push_child(ctxt);
  cscript_statement* init = cscript_vector_at(&f->init_cond_inc, 0, cscript_statement);
  cscript_statement* cond = cscript_vector_at(&f->init_cond_inc, 1, cscript_statement);
  cscript_statement* inc = cscript_vector_at(&f->init_cond_inc, 2, cscript_statement);
  compile_statement(ctxt, state, init);
  int for_loop_cond_start = (int)state->fun->code.vector_size;
  compile_statement(ctxt, state, cond);
  make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_NEQ, state->freereg, 0);
  cscript_instruction i1 = 0;
  CSCRIPT_SET_OPCODE(i1, CSCRIPT_OPCODE_JMP);
  cscript_vector_push_back(ctxt, &state->fun->code, i1, cscript_instruction);
  int for_loop_jump = (int)state->fun->code.vector_size - 1;

  cscript_statement* it = cscript_vector_begin(&f->statements, cscript_statement);
  cscript_statement* it_end = cscript_vector_end(&f->statements, cscript_statement);
  for (; it != it_end; ++it)
    {
    compile_statement(ctxt, state, it);
    }

  compile_statement(ctxt, state, inc);

  cscript_instruction i2 = 0;
  CSCRIPT_SET_OPCODE(i2, CSCRIPT_OPCODE_JMP);
  CSCRIPT_SETARG_sBx(i2, for_loop_cond_start - (int)state->fun->code.vector_size - 1);
  cscript_vector_push_back(ctxt, &state->fun->code, i2, cscript_instruction);

  cscript_instruction* first_jump = cscript_vector_at(&state->fun->code, for_loop_jump, cscript_instruction);
  CSCRIPT_SETARG_sBx(*first_jump, (int)state->fun->code.vector_size - for_loop_jump - 1);
  cscript_environment_pop_child(ctxt);
  }

static void compile_if(cscript_context* ctxt, compiler_state* state, cscript_parsed_if* i)
  {
  cscript_statement* cond = cscript_vector_at(&i->condition, 0, cscript_statement);
  compile_statement(ctxt, state, cond);
  make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_NEQ, state->freereg, 0);
  cscript_instruction i1 = 0;
  CSCRIPT_SET_OPCODE(i1, CSCRIPT_OPCODE_JMP);
  cscript_vector_push_back(ctxt, &state->fun->code, i1, cscript_instruction);
  int if_jump = (int)state->fun->code.vector_size - 1;
  cscript_environment_push_child(ctxt);
  cscript_statement* it = cscript_vector_begin(&i->body, cscript_statement);
  cscript_statement* it_end = cscript_vector_end(&i->body, cscript_statement);
  for (; it != it_end; ++it)
    {
    compile_statement(ctxt, state, it);
    }
  cscript_environment_pop_child(ctxt);
  if (i->alternative.vector_size > 0)
    {
    cscript_instruction i2 = 0;
    CSCRIPT_SET_OPCODE(i2, CSCRIPT_OPCODE_JMP);
    cscript_vector_push_back(ctxt, &state->fun->code, i2, cscript_instruction);
    int else_jump = (int)state->fun->code.vector_size - 1;

    cscript_environment_push_child(ctxt);
    it = cscript_vector_begin(&i->alternative, cscript_statement);
    it_end = cscript_vector_end(&i->alternative, cscript_statement);
    for (; it != it_end; ++it)
      {
      compile_statement(ctxt, state, it);
      }
    cscript_environment_pop_child(ctxt);

    cscript_instruction* alt_jump = cscript_vector_at(&state->fun->code, else_jump, cscript_instruction);
    CSCRIPT_SETARG_sBx(*alt_jump, (int)state->fun->code.vector_size - else_jump - 1);

    cscript_instruction* first_jump = cscript_vector_at(&state->fun->code, if_jump, cscript_instruction);
    CSCRIPT_SETARG_sBx(*first_jump, else_jump - if_jump);
    }
  else
    {
    cscript_instruction* first_jump = cscript_vector_at(&state->fun->code, if_jump, cscript_instruction);
    CSCRIPT_SETARG_sBx(*first_jump, (int)state->fun->code.vector_size - if_jump - 1);
    }
  }

static void compile_statement(cscript_context* ctxt, compiler_state* state, cscript_statement* stmt)
  {
  switch (stmt->type)
    {
    case cscript_statement_type_expression:
      compile_expression(ctxt, state, &stmt->statement.expr);
      break;
    case cscript_statement_type_fixnum:
      compile_fixnum(ctxt, state, &stmt->statement.fixnum);
      break;
    case cscript_statement_type_flonum:
      compile_flonum(ctxt, state, &stmt->statement.flonum);
      break;
    case cscript_statement_type_for:
      compile_for(ctxt, state, &stmt->statement.forloop);
      break;
    case cscript_statement_type_if:
      compile_if(ctxt, state, &stmt->statement.iftest);
      break;
    case cscript_statement_type_comma_separated:
      compile_comma_seperated_statements(ctxt, state, &stmt->statement.stmts);
      break;
    case cscript_statement_type_nop:
      break;
    case cscript_statement_type_assignment:
      compile_assigment(ctxt, state, &stmt->statement.assignment);
      break;
    default:
      cscript_throw(ctxt, CSCRIPT_ERROR_NOT_IMPLEMENTED);
    }
  }

static void compile_parameter(cscript_context* ctxt, cscript_parameter* p, cscript_fixnum pos)
  {
  cscript_environment_entry entry;
  entry.type = CSCRIPT_ENV_TYPE_STACK;
  entry.position = pos;
  switch (p->type)
    {
    case cscript_parameter_type_fixnum:
      entry.register_type = cscript_reg_typeinfo_fixnum;
      break;
    case cscript_parameter_type_flonum:
      entry.register_type = cscript_reg_typeinfo_flonum;
      break;
    case cscript_parameter_type_fixnum_pointer:
      entry.register_type = cscript_reg_typeinfo_fixnum_pointer;
      break;
    case cscript_parameter_type_flonum_pointer:
      entry.register_type = cscript_reg_typeinfo_flonum_pointer;
      break;
    default:
      cscript_assert(0);
      break;
    }
  cscript_string s;
  cscript_string_copy(ctxt, &s, &p->name);
  cscript_environment_add(ctxt, &s, entry);
  }

cscript_function* cscript_compile_program(cscript_context* ctxt, cscript_program* prog)
  {
  cscript_compile_errors_clear(ctxt);
  cscript_function* fun = cscript_function_new(ctxt);
  cscript_parameter* pit = cscript_vector_begin(&prog->parameters, cscript_parameter);
  cscript_parameter* pit_end = cscript_vector_end(&prog->parameters, cscript_parameter);
  cscript_fixnum parameter_pos = 0;
  for (; pit != pit_end; ++pit, ++parameter_pos)
    {
    compile_parameter(ctxt, pit, parameter_pos);
    }
  compiler_state state = init_compiler_state(prog->parameters.vector_size, cscript_reg_typeinfo_fixnum, fun);
  cscript_statement* it = cscript_vector_begin(&prog->statements, cscript_statement);
  cscript_statement* it_end = cscript_vector_end(&prog->statements, cscript_statement);
  for (; it != it_end; ++it)
    {
    compile_statement(ctxt, &state, it);
    }
  fun->result_position = state.freereg;
  return fun;
  }