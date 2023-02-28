#ifndef CSCRIPT_VM_H
#define CSCRIPT_VM_H

#include "cscript.h"
#include "limits.h"
#include "object.h"
#include "parser.h"
#include "func.h"

/*===========================================================================
* 
* Based on the LUA virtual machine
* 
* We assume that instructions are unsigned numbers.
* All instructions have an opcode in the first 6 bits.
* Instructions can have the following fields:
* 'A' : 8 bits
* 'B' : 9 bits
* 'C' : 9 bits
* 'Bx' : 18 bits ('B' and 'C' together)
* 'sBx' : signed Bx
*
* A signed argument is represented in excess K; that is, the number
* value is the unsigned value minus K. K is exactly the maximum value
* for that argument (so that -max is represented by 0, and +max is
* represented by 2*max), which is half the maximum for the corresponding
* unsigned argument.
*==========================================================================*/
/*
** size and position of opcode arguments.
*/
#define CSCRIPT_SIZE_C 8
#define CSCRIPT_SIZE_B 10
#define CSCRIPT_SIZE_Bx (CSCRIPT_SIZE_C + CSCRIPT_SIZE_B)
#define CSCRIPT_SIZE_A 10

#define CSCRIPT_SIZE_OPCODE 4

#define CSCRIPT_POS_C CSCRIPT_SIZE_OPCODE
#define CSCRIPT_POS_B (CSCRIPT_POS_C + CSCRIPT_SIZE_C)
#define CSCRIPT_POS_Bx CSCRIPT_POS_C
#define CSCRIPT_POS_A (CSCRIPT_POS_B + CSCRIPT_SIZE_B)

/*
** limits for opcode arguments.
** we use (signed) int to manipulate most arguments,
** so they must fit in cscript_mem_bits-1 bits (-1 for sign)
*/
#if CSCRIPT_SIZE_Bx < cscript_mem_bits-1
#define CSCRIPT_MAXARG_Bx   ((1<<CSCRIPT_SIZE_Bx)-1)
#define CSCRIPT_MAXARG_sBx  (CSCRIPT_MAXARG_Bx>>1)         /* 'sBx' is signed */
#else
#define CSCRIPT_MAXARG_Bx        MAX_INT
#define CSCRIPT_MAXARG_sBx        MAX_INT
#endif


#define CSCRIPT_MAXARG_A ((1<<CSCRIPT_SIZE_A)-1)
#define CSCRIPT_MAXARG_B ((1<<CSCRIPT_SIZE_B)-1)
#define CSCRIPT_MAXARG_C ((1<<CSCRIPT_SIZE_C)-1)

/* creates a mask with `n' 1 bits at position `p' */
#define CSCRIPT_MASK1(n,p)	((~((~(cscript_instruction)0)<<n))<<p)

/* creates a mask with `n' 0 bits at position `p' */
#define CSCRIPT_MASK0(n,p)	(~CSCRIPT_MASK1(n,p))

/*
** R(x) - register
** Kst(x) - constant (in constant table)
*/

typedef enum 
  {
  CSCRIPT_OPCODE_MOVE,       /*  A B      R(A) := R(B)         */
  CSCRIPT_OPCODE_LOADK,      /*  A Bx	   R(A) := Kst(Bx)      */
  CSCRIPT_OPCODE_SETFIXNUM,  /*  A sBx    R(A) := sBx        */
  CSCRIPT_OPCODE_SETPRIM,    /*  A B      R(A) := B */
  CSCRIPT_OPCODE_MOVETOP,    /*  A B      R(0)..R(B) := R(A)..R(A+B)*/
  CSCRIPT_OPCODE_CALL,       /*  A B C	   R(A) := R(A)(R(A+1+C), ... ,R(A+B+C)) */
  CSCRIPT_OPCODE_EQTYPE,     /*  A B      if (type of R(A) == B) then pc++, else perform the following JMP instruction on the next line*/  
  CSCRIPT_OPCODE_JMP,        /*  sBx      PC += sBx					*/
  CSCRIPT_OPCODE_RETURN,     /*  A B	     return R(A), ... ,R(A+B-1) */
  CSCRIPT_OPCODE_LOADGLOBAL, /*  A Bx     R(A) := Global(Bx) */
  CSCRIPT_OPCODE_STOREGLOBAL,/*  A Bx     Global(Bx) := R(A) */
  } cscript_opcode;

#define CSCRIPT_NUM_OPCODES (cast(int, CSCRIPT_OPCODE_RETURN+1))

#define CSCRIPT_GET_OPCODE(i)	(cast(cscript_opcode, (i)&CSCRIPT_MASK1(CSCRIPT_SIZE_OPCODE,0)))
#define CSCRIPT_SET_OPCODE(i,o)	((i) = (((i)&CSCRIPT_MASK0(CSCRIPT_SIZE_OPCODE,0)) | cast(cscript_instruction, o)))

#define CSCRIPT_GETARG_A(i)	(cast(int, (i)>>CSCRIPT_POS_A))
#define CSCRIPT_SETARG_A(i,u)	((i) = (((i)&CSCRIPT_MASK0(CSCRIPT_SIZE_A,CSCRIPT_POS_A)) | \
		((cast(cscript_instruction, u)<<CSCRIPT_POS_A)&CSCRIPT_MASK1(CSCRIPT_SIZE_A,CSCRIPT_POS_A))))

#define CSCRIPT_GETARG_B(i)	(cast(int, ((i)>>CSCRIPT_POS_B) & CSCRIPT_MASK1(CSCRIPT_SIZE_B,0)))
#define CSCRIPT_SETARG_B(i,b)	((i) = (((i)&CSCRIPT_MASK0(CSCRIPT_SIZE_B,CSCRIPT_POS_B)) | \
		((cast(cscript_instruction, b)<<CSCRIPT_POS_B)&CSCRIPT_MASK1(CSCRIPT_SIZE_B,CSCRIPT_POS_B))))

#define CSCRIPT_GETARG_C(i)	(cast(int, ((i)>>CSCRIPT_POS_C) & CSCRIPT_MASK1(CSCRIPT_SIZE_C,0)))
#define CSCRIPT_SETARG_C(i,b)	((i) = (((i)&CSCRIPT_MASK0(CSCRIPT_SIZE_C,CSCRIPT_POS_C)) | \
		((cast(cscript_instruction, b)<<CSCRIPT_POS_C)&CSCRIPT_MASK1(CSCRIPT_SIZE_C,CSCRIPT_POS_C))))

#define CSCRIPT_GETARG_Bx(i)	(cast(int, ((i)>>CSCRIPT_POS_Bx) & CSCRIPT_MASK1(CSCRIPT_SIZE_Bx,0)))
#define CSCRIPT_SETARG_Bx(i,b)	((i) = (((i)&CSCRIPT_MASK0(CSCRIPT_SIZE_Bx,CSCRIPT_POS_Bx)) | \
		((cast(cscript_instruction, b)<<CSCRIPT_POS_Bx)&CSCRIPT_MASK1(CSCRIPT_SIZE_Bx,CSCRIPT_POS_Bx))))

#define CSCRIPT_GETARG_sBx(i)	(CSCRIPT_GETARG_Bx(i)-CSCRIPT_MAXARG_sBx)
#define CSCRIPT_SETARG_sBx(i,b)	CSCRIPT_SETARG_Bx((i),cast(unsigned int, (b)+CSCRIPT_MAXARG_sBx))


cscript_fixnum* cscript_run(cscript_context* ctxt, cscript_function* fun);
cscript_fixnum* cscript_run_program(cscript_context* ctxt, const cscript_vector* functions);

cscript_string cscript_fun_to_string(cscript_context* ctxt, cscript_function* fun);

#endif //CSCRIPT_VM_H
