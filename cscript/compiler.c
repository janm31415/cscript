#include "compiler.h"
#include "vector.h"
#include "error.h"
#include "vm.h"
#include "object.h"
#include "primitives.h"
#include "context.h"
#include "environment.h"
#include "constant.h"

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
  cscript_memsize freereg;
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
      compile_expression(ctxt, state, cscript_vector_begin(&v->dims, cscript_parsed_expression));
      make_code_abc(ctxt, state->fun, CSCRIPT_OPCODE_MOVE_FROM_ARR, state->freereg, entry.position, state->freereg);
      state->reg_typeinfo = entry.variable_type;
      }
    else
      {
      make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg, entry.position);
      state->reg_typeinfo = entry.variable_type;
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
  cscript_assert(0);
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
    entry.variable_type = cscript_reg_typeinfo_fixnum;
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
    entry.variable_type = cscript_reg_typeinfo_flonum;
    cscript_string s;
    cscript_string_copy(ctxt, &s, &fl->name);
    cscript_environment_add(ctxt, &s, entry);
    cscript_constant_value dim_size = cscript_get_constant_value_expression(dim_expr);
    if (dim_size.type == cscript_number_type_fixnum)
      state->freereg += dim_size.number.fx;
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
    entry.variable_type = cscript_reg_typeinfo_flonum;
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

static void compile_assigment_array(cscript_context* ctxt, compiler_state* state, cscript_parsed_assignment* a, cscript_environment_entry entry)
  {
  cscript_parsed_expression* e = cscript_vector_begin(&a->dims, cscript_parsed_expression);
  compile_expression(ctxt, state, e);
  if (state->reg_typeinfo != cscript_reg_typeinfo_fixnum)
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg, cscript_reg_typeinfo_fixnum);
    state->reg_typeinfo = cscript_reg_typeinfo_fixnum;
    }
  ++state->freereg;
  compile_expression(ctxt, state, &a->expr);
  if (state->reg_typeinfo != entry.variable_type)
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg, entry.variable_type);
    state->reg_typeinfo = entry.variable_type;
    }
  --state->freereg;
  switch (a->op.string_ptr[0])
    {
    case '=':
    {
    make_code_abc(ctxt, state->fun, CSCRIPT_OPCODE_MOVE_TO_ARR, (int)entry.position, state->freereg, state->freereg + 1);
    break;
    }
    default:
      break;
    }
  }

static void compile_assigment_single(cscript_context* ctxt, compiler_state* state, cscript_parsed_assignment* a, cscript_environment_entry entry)
  {
  compile_expression(ctxt, state, &a->expr);
  if (state->reg_typeinfo != entry.variable_type)
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CAST, state->freereg, entry.variable_type);
    state->reg_typeinfo = entry.variable_type;
    }
  switch (a->op.string_ptr[0])
    {
    case '=':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, (int)entry.position, state->freereg);
    break;
    }
    case '+':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg + 1, (int)entry.position);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, entry.variable_type == cscript_reg_typeinfo_flonum ? CSCRIPT_ADD_FLONUM : CSCRIPT_ADD_FIXNUM);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, (int)entry.position, state->freereg);
    break;
    }
    case '*':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg + 1, (int)entry.position);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, entry.variable_type == cscript_reg_typeinfo_flonum ? CSCRIPT_MUL_FLONUM : CSCRIPT_MUL_FIXNUM);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, (int)entry.position, state->freereg);
    break;
    }
    case '-':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg + 1, state->freereg);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg, (int)entry.position);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, entry.variable_type == cscript_reg_typeinfo_flonum ? CSCRIPT_SUB_FLONUM : CSCRIPT_SUB_FIXNUM);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, (int)entry.position, state->freereg);
    break;
    }
    case '/':
    {
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg + 1, state->freereg);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_MOVE, state->freereg, (int)entry.position);
    make_code_ab(ctxt, state->fun, CSCRIPT_OPCODE_CALLPRIM, state->freereg, entry.variable_type == cscript_reg_typeinfo_flonum ? CSCRIPT_DIV_FLONUM : CSCRIPT_DIV_FIXNUM);
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
        compile_assigment_array(ctxt, state, a, entry);
        }
      else if (a->derefence != 0)
        {
        }
      else
        {
        compile_assigment_single(ctxt, state, a, entry);
        }
      }
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

cscript_function* cscript_compile_statement(cscript_context* ctxt, cscript_statement* stmt)
  {
  cscript_compile_errors_clear(ctxt);
  cscript_function* fun = cscript_function_new(ctxt);
  compiler_state state = init_compiler_state(0, cscript_reg_typeinfo_fixnum, fun);
  compile_statement(ctxt, &state, stmt);
  return fun;
  }

cscript_function* cscript_compile_program(cscript_context* ctxt, cscript_program* prog)
  {
  cscript_compile_errors_clear(ctxt);
  cscript_function* fun = cscript_function_new(ctxt);
  compiler_state state = init_compiler_state(0, cscript_reg_typeinfo_fixnum, fun);
  cscript_statement* it = cscript_vector_begin(&prog->statements, cscript_statement);
  cscript_statement* it_end = cscript_vector_end(&prog->statements, cscript_statement);
  for (; it != it_end; ++it)
    {
    compile_statement(ctxt, &state, it);
    }
  fun->result_position = state.freereg;
  return fun;
  }