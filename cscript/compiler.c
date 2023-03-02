#include "compiler.h"
#include "vector.h"
#include "error.h"
#include "vm.h"
#include "object.h"
#include "primitives.h"

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
  int* op_it_end = cscript_vector_end(&e->fops, int);
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
  int* op_it_end = cscript_vector_end(&e->fops, int);
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
  int* op_it_end = cscript_vector_end(&e->fops, int);
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

static void compile_comma_seperated_statements(cscript_context* ctxt, compiler_state* state, cscript_vector* stmts)
  {
  cscript_statement* it = cscript_vector_begin(stmts, cscript_statement);
  cscript_statement* it_end = cscript_vector_end(stmts, cscript_statement);
  for (; it != it_end; ++it)
    {
    compile_statement(ctxt, state, it);
    }
  }


static void compile_fixnum(cscript_context* ctxt, compiler_state* state, cscript_parsed_fixnum* fx)
  {
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
    case cscript_statement_type_comma_separated:
      compile_comma_seperated_statements(ctxt, state, &stmt->statement.stmts);
      break;
    case cscript_statement_type_nop:
      break;
    default:
      cscript_throw(ctxt, CSCRIPT_ERROR_NOT_IMPLEMENTED);
    }
  }

cscript_function* cscript_compile_statement(cscript_context* ctxt, cscript_statement* stmt)
  {
  cscript_compile_errors_clear(ctxt);
  cscript_function* fun = cscript_function_new(ctxt);
  compiler_state state = init_compiler_state(0, cscript_reg_typeinfo_fixnum, fun);
  compile_statement(ctxt, &state, stmt);
  return fun;
  }

cscript_vector cscript_compile_program(cscript_context* ctxt, cscript_program* prog)
  {
  cscript_vector compiled;
  cscript_vector_init(ctxt, &compiled, cscript_function*);
  cscript_statement* it = cscript_vector_begin(&prog->statements, cscript_statement);
  cscript_statement* it_end = cscript_vector_end(&prog->statements, cscript_statement);
  for (; it != it_end; ++it)
    {
    cscript_function* fun = cscript_compile_statement(ctxt, it);
    cscript_vector_push_back(ctxt, &compiled, fun, cscript_function*);
    }
  return compiled;
  }

void cscript_compiled_program_destroy(cscript_context* ctxt, cscript_vector* compiled_program)
  {
  cscript_function** it = cscript_vector_begin(compiled_program, cscript_function*);
  cscript_function** it_end = cscript_vector_end(compiled_program, cscript_function*);
  for (; it != it_end; ++it)
    {
    cscript_function_free(ctxt, *it);
    }
  cscript_vector_destroy(ctxt, compiled_program);
  }