#include "object.h"
#include "context.h"
#include "syscalls.h"
#include <string.h>
#include <stdio.h>

int cscript_log2(uint32_t x)
  {
  static const cscript_byte log_8[255] = {
    0,
    1,1,
    2,2,2,2,
    3,3,3,3,3,3,3,3,
    4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
    5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
    };
  if (x >= 0x00010000)
    {
    if (x >= 0x01000000)
      return log_8[((x >> 24) & 0xff) - 1] + 24;
    else
      return log_8[((x >> 16) & 0xff) - 1] + 16;
    }
  else {
    if (x >= 0x00000100)
      return log_8[((x >> 8) & 0xff) - 1] + 8;
    else if (x)
      return log_8[(x & 0xff) - 1];
    return -1;  /* special `log' for 0 */
    }
  }

int cscript_objects_equal(cscript_context* ctxt, const cscript_object* obj1, const cscript_object* obj2)
  {
  if (cscript_object_get_type(obj1) != cscript_object_get_type(obj2))
    return 0;
  switch (cscript_object_get_type(obj1))
    {
    case cscript_object_type_undefined:
      return 1;
    case cscript_object_type_fixnum:
      return obj1->value.fx == obj2->value.fx;
    case cscript_object_type_flonum:
      return obj1->value.fl == obj2->value.fl;
    case cscript_object_type_string:
      return strcmp(obj1->value.s.string_ptr, obj2->value.s.string_ptr) == 0 ? 1 : 0;
    default:
      //if (obj1->value.v.vector_size != obj2->value.v.vector_size)
        return 0;
      //return cscript_objects_equal_recursive(ctxt, obj1, obj2);
    }
  }

cscript_object make_cscript_object_fixnum(cscript_fixnum fx)
  {
  cscript_object obj;
  obj.type = cscript_object_type_fixnum;
  obj.value.fx = fx;
  return obj;
  }

cscript_object make_cscript_object_flonum(cscript_flonum fl)
  {
  cscript_object obj;
  obj.type = cscript_object_type_flonum;
  obj.value.fl = fl;
  return obj;
  }

cscript_object make_cscript_object_string(cscript_context* ctxt, const char* s)
  {
  cscript_object obj;
  obj.type = cscript_object_type_string;
  cscript_string_init(ctxt, &(obj.value.s), s);
  return obj;
  }

void cscript_object_destroy(cscript_context* ctxt, cscript_object* obj)
  {
  switch (cscript_object_get_type(obj))
    {   
    case cscript_object_type_string:
    {
    cscript_string_destroy(ctxt, &(obj->value.s));
    break;
    }
    default:
      break;
    }
  }

typedef struct cscript_runtime_task
  {
  const char* text;
  cscript_object* obj;
  int second_item_of_pair;
  } cscript_runtime_task;

static cscript_runtime_task make_empty_task()
  {
  cscript_runtime_task task;
  task.text = 0;
  task.obj = 0;
  task.second_item_of_pair = 0;
  return task;
  }

static cscript_runtime_task make_object_task(cscript_object* obj)
  {
  cscript_runtime_task task;
  task.text = 0;
  task.obj = obj;
  task.second_item_of_pair = 0;
  return task;
  }

static cscript_runtime_task make_text_task(const char* txt)
  {
  cscript_runtime_task task;
  task.text = txt;
  task.obj = 0;
  task.second_item_of_pair = 0;
  return task;
  }

void cscript_object_append_to_string(cscript_context* ctxt, cscript_object* input_obj, cscript_string* s, int display)
  {
  cscript_vector tasks;
  cscript_vector_init(ctxt, &tasks, cscript_runtime_task);
  cscript_runtime_task first_task = make_object_task(input_obj);
  cscript_vector_push_back(ctxt, &tasks, first_task, cscript_runtime_task);
  while (tasks.vector_size > 0)
    {
    cscript_runtime_task current_task = *cscript_vector_back(&tasks, cscript_runtime_task);
    cscript_vector_pop_back(&tasks);
    if (current_task.text != 0)
      {
      cscript_string_append_cstr(ctxt, s, current_task.text);
      }
    else
      {
      switch (cscript_object_get_type(current_task.obj))
        {
        case cscript_object_type_undefined:
          cscript_string_append_cstr(ctxt, s, "#undefined");
          break;       
        case cscript_object_type_fixnum:
        {
        char str[256];
        cscript_fixnum_to_char(str, current_task.obj->value.fx);
        cscript_string_append_cstr(ctxt, s, str);
        break;
        }
        case cscript_object_type_flonum:
        {
        char str[256];
        cscript_flonum_to_char(str, current_task.obj->value.fl);
        cscript_string_append_cstr(ctxt, s, str);
        break;
        }     
        }
      }
    }

  cscript_vector_destroy(ctxt, &tasks);
  }

cscript_string cscript_object_to_string(cscript_context* ctxt, cscript_object* input_obj, int display)
  {
  cscript_string s;
  cscript_string_init(ctxt, &s, "");
  cscript_object_append_to_string(ctxt, input_obj, &s, display);
  return s;
  }