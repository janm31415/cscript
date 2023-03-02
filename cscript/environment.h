#ifndef CSCRIPT_ENVIRONMENT_H
#define CSCRIPT_ENVIRONMENT_H

#include "cscript.h"
#include "string.h"
#include "limits.h"
#include "vector.h"
#include "object.h"

typedef enum
  {
  CSCRIPT_ENV_TYPE_STACK,
  CSCRIPT_ENV_TYPE_GLOBAL
  } cscript_environment_type;

typedef struct cscript_environment_entry
  {
  cscript_environment_type type;
  cscript_fixnum position;
  } cscript_environment_entry;

void cscript_environment_init(cscript_context* ctxt);
void cscript_environment_destroy(cscript_context* ctxt);

void cscript_environment_add(cscript_context* ctxt, cscript_string* name, cscript_environment_entry entry);
void cscript_environment_add_to_base(cscript_context* ctxt, cscript_string* name, cscript_environment_entry entry);
int cscript_environment_find(cscript_environment_entry* entry, cscript_context* ctxt, cscript_string* name);
int cscript_environment_find_recursive(cscript_environment_entry* entry, cscript_context* ctxt, cscript_string* name);
void cscript_environment_update(cscript_context* ctxt, cscript_string* name, cscript_environment_entry entry);

void cscript_environment_push_child(cscript_context* ctxt);
void cscript_environment_pop_child(cscript_context* ctxt);

// The next 2 functions are useful for garbage collection where we need to run over the environment in order to know which heap positions to keep.

// returns the number of elements in the environment at the parent/base level (thus not taking into account children)
cscript_memsize cscript_environment_base_size(cscript_context* ctxt);

// returns 1 if a valid environment entry found at position pos, 0 otherwise. The entry is returned in 'entry'.
int cscript_environment_base_at(cscript_environment_entry* entry, cscript_string* name, cscript_context* ctxt, cscript_memsize pos);

cscript_object* cscript_environment_find_key_given_position(cscript_context* ctxt, cscript_fixnum global_position);

//cscript_string cscript_show_environment(cscript_context* ctxt);

#endif //CSCRIPT_ENVIRONMENT_H