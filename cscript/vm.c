#include "vm.h"
#include "context.h"
#include "error.h"
#include "map.h"
#include "primitives.h"
#include "environment.h"
#include "syscalls.h"
#include "limits.h"

#include <stdio.h>
#include <string.h>
#include <math.h>

static cscript_string instruction_to_string(cscript_context* ctxt, cscript_instruction instruc)
  {
  cscript_string s;
  cscript_string_init(ctxt, &s, "");
  char buffer[256];
  switch (CSCRIPT_GET_OPCODE(instruc))
    {
    case CSCRIPT_OPCODE_MOVE:
    {
    const int a = CSCRIPT_GETARG_A(instruc);
    const int b = CSCRIPT_GETARG_B(instruc);
    cscript_string_append_cstr(ctxt, &s, "MOVE R(");
    cscript_int_to_char(buffer, a);
    cscript_string_append_cstr(ctxt, &s, buffer);
    cscript_string_append_cstr(ctxt, &s, ") := R(");
    cscript_int_to_char(buffer, b);
    cscript_string_append_cstr(ctxt, &s, buffer);
    cscript_string_append_cstr(ctxt, &s, ")");
    break;
    }
    case CSCRIPT_OPCODE_LOADGLOBAL:
    {
    const int a = CSCRIPT_GETARG_A(instruc);
    const int b = CSCRIPT_GETARG_Bx(instruc);
    cscript_string_append_cstr(ctxt, &s, "LOADGLOBAL R(");
    cscript_int_to_char(buffer, a);
    cscript_string_append_cstr(ctxt, &s, buffer);
    cscript_string_append_cstr(ctxt, &s, ") := GLOBAL(");
    cscript_int_to_char(buffer, b);
    cscript_string_append_cstr(ctxt, &s, buffer);
    cscript_string_append_cstr(ctxt, &s, ")");
    break;
    }
    case CSCRIPT_OPCODE_STOREGLOBAL:
    {
    const int a = CSCRIPT_GETARG_A(instruc);
    const int b = CSCRIPT_GETARG_Bx(instruc);
    cscript_string_append_cstr(ctxt, &s, "STOREGLOBAL GLOBAL(");
    cscript_int_to_char(buffer, b);
    cscript_string_append_cstr(ctxt, &s, buffer);
    cscript_string_append_cstr(ctxt, &s, ") := R(");
    cscript_int_to_char(buffer, a);
    cscript_string_append_cstr(ctxt, &s, buffer);
    cscript_string_append_cstr(ctxt, &s, ")");
    break;
    }
    case CSCRIPT_OPCODE_MOVETOP:
    {
    const int a = CSCRIPT_GETARG_A(instruc);
    const int b = CSCRIPT_GETARG_B(instruc);
    cscript_string_append_cstr(ctxt, &s, "MOVETOP R(0)..R(");
    cscript_int_to_char(buffer, b);
    cscript_string_append_cstr(ctxt, &s, buffer);
    cscript_string_append_cstr(ctxt, &s, ") := R(");
    cscript_int_to_char(buffer, a);
    cscript_string_append_cstr(ctxt, &s, buffer);
    cscript_string_append_cstr(ctxt, &s, ")..R(");
    cscript_int_to_char(buffer, a + b);
    cscript_string_append_cstr(ctxt, &s, buffer);
    cscript_string_append_cstr(ctxt, &s, ")   R(");
    cscript_int_to_char(buffer, b + 1);
    cscript_string_append_cstr(ctxt, &s, buffer);
    cscript_string_append_cstr(ctxt, &s, ")->type = blocking");
    break;
    }
    case CSCRIPT_OPCODE_LOADK:
    {
    const int a = CSCRIPT_GETARG_A(instruc);
    const int b = CSCRIPT_GETARG_Bx(instruc);
    cscript_string_append_cstr(ctxt, &s, "LOADK R(");
    cscript_int_to_char(buffer, a);
    cscript_string_append_cstr(ctxt, &s, buffer);
    cscript_string_append_cstr(ctxt, &s, ") := K(");
    cscript_int_to_char(buffer, b);
    cscript_string_append_cstr(ctxt, &s, buffer);
    cscript_string_append_cstr(ctxt, &s, ")");
    break;
    }
    case CSCRIPT_OPCODE_SETFIXNUM:
    {
    const int a = CSCRIPT_GETARG_A(instruc);
    const int b = CSCRIPT_GETARG_sBx(instruc);
    cscript_string_append_cstr(ctxt, &s, "SETFIXNUM R(");
    cscript_int_to_char(buffer, a);
    cscript_string_append_cstr(ctxt, &s, buffer);
    cscript_string_append_cstr(ctxt, &s, ") := ");
    cscript_int_to_char(buffer, b);
    cscript_string_append_cstr(ctxt, &s, buffer);
    break;
    }
    case CSCRIPT_OPCODE_CALLPRIM:
    {
    const int a = CSCRIPT_GETARG_A(instruc);
    cscript_string_append_cstr(ctxt, &s, "CALLPRIM R(");
    cscript_int_to_char(buffer, a);
    cscript_string_append_cstr(ctxt, &s, buffer);
    cscript_string_append_cstr(ctxt, &s, ")");
    break;
    }
    case CSCRIPT_OPCODE_EQTYPE:
    {
    const int a = CSCRIPT_GETARG_A(instruc);
    const int b = CSCRIPT_GETARG_B(instruc);
    cscript_string_append_cstr(ctxt, &s, "EQTYPE if R(");
    cscript_int_to_char(buffer, a);
    cscript_string_append_cstr(ctxt, &s, buffer);
    cscript_string_append_cstr(ctxt, &s, ")->type == ");
    cscript_int_to_char(buffer, b);
    cscript_string_append_cstr(ctxt, &s, buffer);
    cscript_string_append_cstr(ctxt, &s, " then skip next line");
    break;
    }
    case CSCRIPT_OPCODE_JMP:
    {
    int b = CSCRIPT_GETARG_sBx(instruc);
    cscript_string_append_cstr(ctxt, &s, "JUMP ");
    cscript_int_to_char(buffer, b);
    cscript_string_append_cstr(ctxt, &s, buffer);
    break;
    }
    case CSCRIPT_OPCODE_RETURN:
    {
    const int a = CSCRIPT_GETARG_A(instruc);
    cscript_string_append_cstr(ctxt, &s, "RETURN R(");
    cscript_int_to_char(buffer, a);
    cscript_string_append_cstr(ctxt, &s, buffer);
    cscript_string_append_cstr(ctxt, &s, ")");
    break;
    }   
    default:
      break;
    }
  return s;
  }


cscript_fixnum* cscript_run(cscript_context* ctxt, cscript_function* fun)
  {
  cscript_assert(fun != NULL);
  const cscript_instruction* pc = cscript_vector_begin(&(fun)->code, cscript_instruction);  
  const cscript_instruction* pc_end = cscript_vector_end(&(fun)->code, cscript_instruction);
  while (pc < pc_end)
    {
    cscript_assert(pc < cscript_vector_end(&(fun)->code, cscript_instruction));
    const cscript_instruction instruc = *pc++;
    const int opcode = CSCRIPT_GET_OPCODE(instruc);
    cscript_assert((opcode == CSCRIPT_OPCODE_JMP) || (CSCRIPT_GETARG_A(instruc) < cscript_maxstack));
    switch (opcode)
      {
      case CSCRIPT_OPCODE_MOVE:
      {
      const int a = CSCRIPT_GETARG_A(instruc);
      const int b = CSCRIPT_GETARG_B(instruc);
      memcpy(((cscript_fixnum*)ctxt->stack.vector_ptr)+a, ((cscript_fixnum*)ctxt->stack.vector_ptr) + b, sizeof(cscript_fixnum));
      continue;
      }
      case CSCRIPT_OPCODE_MOVETOP:
      {
      const int a = CSCRIPT_GETARG_A(instruc);
      const int b = CSCRIPT_GETARG_B(instruc);
      for (int j = 0; j <= b; ++j)
        {
        memcpy(((cscript_fixnum*)ctxt->stack.vector_ptr) + j, ((cscript_fixnum*)ctxt->stack.vector_ptr) + a + j, sizeof(cscript_fixnum));
        }
      continue;
      }
      case CSCRIPT_OPCODE_LOADGLOBAL:
      {
      const int a = CSCRIPT_GETARG_A(instruc);
      const int bx = CSCRIPT_GETARG_Bx(instruc);
      memcpy(((cscript_fixnum*)ctxt->stack.vector_ptr) + a, ((cscript_fixnum*)ctxt->globals.vector_ptr) + bx, sizeof(cscript_fixnum));
      continue;
      }
      case CSCRIPT_OPCODE_STOREGLOBAL:
      {
      const int a = CSCRIPT_GETARG_A(instruc);
      const int bx = CSCRIPT_GETARG_Bx(instruc);
      memcpy(((cscript_fixnum*)ctxt->globals.vector_ptr) + bx, ((cscript_fixnum*)ctxt->stack.vector_ptr) + a, sizeof(cscript_fixnum));
      continue;
      }
      case CSCRIPT_OPCODE_LOADK:
      {
      const int a = CSCRIPT_GETARG_A(instruc);
      const int bx = CSCRIPT_GETARG_Bx(instruc);
      cscript_fixnum* target = cscript_vector_at(&ctxt->stack, a, cscript_fixnum);
      const cscript_fixnum* k = cscript_vector_at(&(fun)->constants, bx, cscript_fixnum);
      memcpy(target, k, sizeof(cscript_fixnum));
      continue;
      }
      case CSCRIPT_OPCODE_SETFIXNUM:
      {
      const int a = CSCRIPT_GETARG_A(instruc);
      cscript_fixnum* target = cscript_vector_at(&ctxt->stack, a, cscript_fixnum);
      const int b = CSCRIPT_GETARG_sBx(instruc);
      *target = cast(cscript_fixnum, b);
      continue;
      }    
      case CSCRIPT_OPCODE_CALLPRIM:
      {
      const int a = CSCRIPT_GETARG_A(instruc);
      const int b = CSCRIPT_GETARG_B(instruc);
      cscript_call_primitive(ctxt, b, a);
      continue;
      }
      case CSCRIPT_OPCODE_CAST:
      {
      const int a = CSCRIPT_GETARG_A(instruc);
      const int b = CSCRIPT_GETARG_B(instruc);
      if (b == 1)
        {
        cscript_flonum ra = cast(cscript_flonum, *cscript_vector_at(&ctxt->stack, a, cscript_fixnum));
        memcpy(cscript_vector_at(&ctxt->stack, a, cscript_fixnum), &ra, sizeof(cscript_fixnum));
        }
      else
        {
        cscript_fixnum ra = cast(cscript_fixnum, *cscript_vector_at(&ctxt->stack, a, cscript_flonum));
        memcpy(cscript_vector_at(&ctxt->stack, a, cscript_fixnum), &ra, sizeof(cscript_fixnum));
        }
      continue;
      }
      case CSCRIPT_OPCODE_EQTYPE:
      {
      /*
      const int a = CSCRIPT_GETARG_A(instruc);
      const int b = CSCRIPT_GETARG_B(instruc);
      const cscript_object* ra = cscript_vector_at(&ctxt->stack, a, cscript_object);
      if (cscript_object_get_type(ra) == b)
        {
        ++pc;
        }
      else
        {
        const cscript_instruction next_i = *pc++;
        cscript_assert(CSCRIPT_GET_OPCODE(next_i) == CSCRIPT_OPCODE_JMP);
        const int offset = CSCRIPT_GETARG_sBx(next_i);
        pc += offset;
        }
        */
      continue;
      }
      case CSCRIPT_OPCODE_JMP:
      {
      const int sbx = CSCRIPT_GETARG_sBx(instruc);
      pc += sbx;
      continue;
      }
      case CSCRIPT_OPCODE_RETURN:
      {
      const int a = CSCRIPT_GETARG_A(instruc);
      const int b = CSCRIPT_GETARG_B(instruc);
      if (a != 0)
        {
        for (int j = 0; j < b; ++j)
          {
          cscript_fixnum* retj = cscript_vector_at(&ctxt->stack, j, cscript_fixnum);
          const cscript_fixnum* srcj = cscript_vector_at(&ctxt->stack, a + j, cscript_fixnum);
          *retj = *srcj;
          }
        }      
      return cscript_vector_at(&ctxt->stack, 0, cscript_fixnum);
      }
      default:
        cscript_throw(ctxt, CSCRIPT_ERROR_NOT_IMPLEMENTED);
      }
    }
  //return cscript_vector_at(&ctxt->stack, 0, cscript_object);
  }

cscript_string cscript_fun_to_string(cscript_context* ctxt, cscript_function* fun)
  {
  cscript_string s;
  cscript_string_init(ctxt, &s, "");
  cscript_instruction* it = cscript_vector_begin(&fun->code, cscript_instruction);
  cscript_instruction* it_end = cscript_vector_end(&fun->code, cscript_instruction);
  for (; it != it_end; ++it)
    {
    const cscript_instruction instruc = *it;
    cscript_string tmp = instruction_to_string(ctxt, instruc);
    cscript_string_append(ctxt, &s, &tmp);
    cscript_string_destroy(ctxt, &tmp);
    cscript_string_append_cstr(ctxt, &s, "\n");
    }
  return s;
  }

void cscript_show_stack(cscript_context* ctxt, cscript_string* s, int stack_start, int stack_end)
  {
  char buffer[256];
  cscript_string_append_cstr(ctxt, s, "CSCRIPT STACK:\n");
  if (stack_end >= cscript_maxstack)
    stack_end = cscript_maxstack - 1;
  for (int i = stack_start; i <= stack_end; ++i)
    {
    cscript_int_to_char(buffer, i);
    cscript_string_append_cstr(ctxt, s, "  ");
    cscript_string_append_cstr(ctxt, s, buffer);
    cscript_string_append_cstr(ctxt, s, ": ");
    cscript_object* stack_item = cscript_vector_at(&ctxt->stack, i, cscript_object);
    cscript_string tmp = cscript_object_to_string(ctxt, stack_item, 0);
    cscript_string_append(ctxt, s, &tmp);
    cscript_string_destroy(ctxt, &tmp);
    cscript_string_push_back(ctxt, s, '\n');
    }  
  }

cscript_fixnum* cscript_run_program(cscript_context* ctxt, const cscript_vector* functions)
  {
  cscript_function** it = cscript_vector_begin(functions, cscript_function*);
  cscript_function** it_end = cscript_vector_end(functions, cscript_function*);
  for (; it != it_end; ++it)
    {
    cscript_run(ctxt, *it);
    }
  return cscript_vector_at(&ctxt->stack, 0, cscript_fixnum);
  }
