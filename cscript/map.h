#ifndef CSCRIPT_MAP_H
#define CSCRIPT_MAP_H

/*
Based on table from LUA
*/

#include "cscript.h"
#include "object.h"
#include "limits.h"

typedef struct cscript_map_node 
  {
  cscript_object key;
  cscript_object value;
  struct cscript_map_node* next; 
  } cscript_map_node;


typedef struct cscript_map 
  {
  cscript_memsize log_node_size;  /* log2 of size of `node' array */
  cscript_object* array;  /* array part */
  cscript_map_node* node;
  cscript_map_node* first_free;  /* this position is free; all positions after it are full */
  cscript_memsize array_size;  /* size of `array' array */
  } cscript_map;


#define node_size(m) (1<<((m)->log_node_size))
#define get_node(m, i)  (&(m)->node[i])
#define get_key(n) (&(n)->key)
#define get_value(n) (&(n)->value)

cscript_map* cscript_map_new(cscript_context* ctxt, cscript_memsize array_size, cscript_memsize log_node_size);
void cscript_map_free(cscript_context* ctxt, cscript_map* map);
// the keys could be used by someone else so they are not destroyed by default
void cscript_map_keys_free(cscript_context* ctxt, cscript_map* map);
// the values could be used by someone else so they are not destroyed by default
void cscript_map_values_free(cscript_context* ctxt, cscript_map* map);
cscript_object* cscript_map_insert_indexed(cscript_context* ctxt, cscript_map* map, cscript_memsize index);
cscript_object* cscript_map_insert(cscript_context* ctxt, cscript_map* map, const cscript_object* key);
cscript_object* cscript_map_insert_string(cscript_context* ctxt, cscript_map* map, const char* str);
cscript_object* cscript_map_get_indexed(cscript_map* map, cscript_memsize index);
cscript_object* cscript_map_get(cscript_context* ctxt, cscript_map* map, const cscript_object* key);
cscript_object* cscript_map_get_string(cscript_map* map, const char* str);

#endif //CSCRIPT_MAP_H