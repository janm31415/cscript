#include "foreign.h"
#include "context.h"
#include "error.h"

static cscript_external_function external_function_init(cscript_context* ctxt, const char* name, void* address, cscript_foreign_return_type ret_type)
  {
  cscript_external_function ext;
  cscript_string_init(ctxt, &ext.name, name);
  ext.address = address;
  ext.return_type = ret_type;
  return ext;
  }

static void register_external_function(cscript_context* ctxt, cscript_external_function* ext)
  {
  cscript_object key;
  key.type = cscript_object_type_string;
  key.value.s = ext->name;
  cscript_object* pos = cscript_map_insert(ctxt, ctxt->externals_map, &key);
  pos->type = cscript_object_type_fixnum;
  pos->value.fx = cast(cscript_fixnum, ctxt->externals.vector_size);
  cscript_vector_push_back(ctxt, &ctxt->externals, *ext, cscript_external_function);
  }

void cscript_register_external_function(cscript_context* ctxt, const char* name, void* address, cscript_foreign_return_type ret_type)
  {
  cscript_external_function ext = external_function_init(ctxt,name, address, ret_type);
  register_external_function(ctxt, &ext);
  }

void cscript_external_function_destroy(cscript_context* ctxt, cscript_external_function* ext)
  {
  cscript_string_destroy(ctxt, &ext->name);
  }

static void* get_argument_pointer(cscript_context* ctxt, int stack_offset)
  {
  cscript_fixnum* fx = cscript_vector_at(&ctxt->stack, stack_offset, cscript_fixnum);
  return fx;
  }

#define CSCRIPT_CALL_EXTERNAL(funargs, args) \
    switch (ext->return_type) \
      { \
      case cscript_foreign_fixnum: \
      { \
      typedef cscript_fixnum(*fun)funargs; \
      obj.type = cscript_object_type_fixnum; \
      obj.value.fx = (cast(fun, ext->address))args; \
      break; \
      } \
      case cscript_foreign_flonum: \
      { \
      typedef cscript_flonum(*fun)funargs; \
      obj.type = cscript_object_type_flonum; \
      obj.value.fl = (cast(fun, ext->address))args; \
      break; \
      } \
      case cscript_foreign_void: \
      { \
      typedef void (*fun)funargs; \
      obj.type = cscript_object_type_void; \
      (cast(fun, ext->address))args; \
      break; \
      } \
      }

cscript_object cscript_call_external(cscript_context* ctxt, cscript_external_function* ext, int argument_stack_offset, int nr_of_args)
  {
  cscript_object obj;
  obj.type = cscript_object_type_undefined;
  switch (nr_of_args)
    {
    case 0:
    {
    CSCRIPT_CALL_EXTERNAL((), ());
    break;
    }
    case 1:
    {
    void* arg1 = get_argument_pointer(ctxt, argument_stack_offset);
    CSCRIPT_CALL_EXTERNAL((void*), (arg1));
    break;
    }
    case 2:
    {
    void* arg1 = get_argument_pointer(ctxt, argument_stack_offset);
    void* arg2 = get_argument_pointer(ctxt, argument_stack_offset + 1);
    CSCRIPT_CALL_EXTERNAL((void*, void*), (arg1, arg2));
    break;
    }
    case 3:
    {
    void* arg1 = get_argument_pointer(ctxt, argument_stack_offset);
    void* arg2 = get_argument_pointer(ctxt, argument_stack_offset + 1);
    void* arg3 = get_argument_pointer(ctxt, argument_stack_offset + 2);
    CSCRIPT_CALL_EXTERNAL((void*, void*, void*), (arg1, arg2, arg3));
    break;
    }
    case 4:
    {
    void* arg1 = get_argument_pointer(ctxt, argument_stack_offset);
    void* arg2 = get_argument_pointer(ctxt, argument_stack_offset + 1);
    void* arg3 = get_argument_pointer(ctxt, argument_stack_offset + 2);
    void* arg4 = get_argument_pointer(ctxt, argument_stack_offset + 3);
    CSCRIPT_CALL_EXTERNAL((void*, void*, void*, void*), (arg1, arg2, arg3, arg4));
    break;
    }
    case 5:
    {
    void* arg1 = get_argument_pointer(ctxt, argument_stack_offset);
    void* arg2 = get_argument_pointer(ctxt, argument_stack_offset + 1);
    void* arg3 = get_argument_pointer(ctxt, argument_stack_offset + 2);
    void* arg4 = get_argument_pointer(ctxt, argument_stack_offset + 3);
    void* arg5 = get_argument_pointer(ctxt, argument_stack_offset + 4);
    CSCRIPT_CALL_EXTERNAL((void*, void*, void*, void*, void*), (arg1, arg2, arg3, arg4, arg5));
    break;
    }
    case 6:
    {
    void* arg1 = get_argument_pointer(ctxt, argument_stack_offset);
    void* arg2 = get_argument_pointer(ctxt, argument_stack_offset + 1);
    void* arg3 = get_argument_pointer(ctxt, argument_stack_offset + 2);
    void* arg4 = get_argument_pointer(ctxt, argument_stack_offset + 3);
    void* arg5 = get_argument_pointer(ctxt, argument_stack_offset + 4);
    void* arg6 = get_argument_pointer(ctxt, argument_stack_offset + 5);
    CSCRIPT_CALL_EXTERNAL((void*, void*, void*, void*, void*, void*), (arg1, arg2, arg3, arg4, arg5, arg6));
    break;
    }
    case 7:
    {
    void* arg1 = get_argument_pointer(ctxt, argument_stack_offset);
    void* arg2 = get_argument_pointer(ctxt, argument_stack_offset + 1);
    void* arg3 = get_argument_pointer(ctxt, argument_stack_offset + 2);
    void* arg4 = get_argument_pointer(ctxt, argument_stack_offset + 3);
    void* arg5 = get_argument_pointer(ctxt, argument_stack_offset + 4);
    void* arg6 = get_argument_pointer(ctxt, argument_stack_offset + 5);
    void* arg7 = get_argument_pointer(ctxt, argument_stack_offset + 6);
    CSCRIPT_CALL_EXTERNAL((void*, void*, void*, void*, void*, void*, void*), (arg1, arg2, arg3, arg4, arg5, arg6, arg7));
    break;
    }
    case 8:
    {
    void* arg1 = get_argument_pointer(ctxt, argument_stack_offset);
    void* arg2 = get_argument_pointer(ctxt, argument_stack_offset + 1);
    void* arg3 = get_argument_pointer(ctxt, argument_stack_offset + 2);
    void* arg4 = get_argument_pointer(ctxt, argument_stack_offset + 3);
    void* arg5 = get_argument_pointer(ctxt, argument_stack_offset + 4);
    void* arg6 = get_argument_pointer(ctxt, argument_stack_offset + 5);
    void* arg7 = get_argument_pointer(ctxt, argument_stack_offset + 6);
    void* arg8 = get_argument_pointer(ctxt, argument_stack_offset + 7);
    CSCRIPT_CALL_EXTERNAL((void*, void*, void*, void*, void*, void*, void*, void*), (arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8));
    break;
    }
    default:
    {
    cscript_runtime_error_cstr(ctxt, CSCRIPT_ERROR_INVALID_NUMBER_OF_ARGUMENTS, -1, -1, "Too many parameters for external call");
    break;
    }
    }
  return obj;
  }