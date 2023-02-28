#include "environment.h"
#include "map.h"
#include "vector.h"
#include "context.h"
#include "map.h"
#include "object.h"

void cscript_environment_init(cscript_context* ctxt)
  {
  cscript_vector_init(ctxt, &ctxt->environment, cscript_map*);
  cscript_map* default_environment = cscript_map_new(ctxt, 0, 3);
  cscript_vector_push_back(ctxt, &ctxt->environment, default_environment, cscript_map*);
  }

void cscript_environment_destroy(cscript_context* ctxt)
  {
  cscript_map** map_it = cscript_vector_begin(&ctxt->environment, cscript_map*);
  cscript_map** map_it_end = cscript_vector_end(&ctxt->environment, cscript_map*);
  for (; map_it != map_it_end; ++map_it)
    {
    cscript_map_keys_free(ctxt, *map_it);
    cscript_map_free(ctxt, *map_it);
    }
  cscript_vector_destroy(ctxt, &ctxt->environment);
  }

void cscript_environment_add(cscript_context* ctxt, cscript_string* name, cscript_environment_entry entry)
  {
  cscript_assert(ctxt->environment.vector_size > 0);
  cscript_map** active_map = cscript_vector_back(&ctxt->environment, cscript_map*);
  cscript_object key;
  key.type = cscript_object_type_string;
  key.value.s = *name;
  cscript_object* entry_object = cscript_map_insert(ctxt, *active_map, &key);
  //abusing cscript_object type fo fit in cscript_environment_entry, but avoiding 0
  entry_object->type = cast(int, entry.type) + 1;
  entry_object->value.fx = entry.position;
  }

void cscript_environment_add_to_base(cscript_context* ctxt, cscript_string* name, cscript_environment_entry entry)
  {
  cscript_assert(ctxt->environment.vector_size > 0);
  cscript_map** active_map = cscript_vector_begin(&ctxt->environment, cscript_map*);
  cscript_object key;
  key.type = cscript_object_type_string;
  key.value.s = *name;
  cscript_object* entry_object = cscript_map_insert(ctxt, *active_map, &key);
  //abusing cscript_object type fo fit in cscript_environment_entry, but avoiding 0
  entry_object->type = cast(int, entry.type) + 1;
  entry_object->value.fx = entry.position;
  }

int cscript_environment_find(cscript_environment_entry* entry, cscript_context* ctxt, cscript_string* name)
  {
  cscript_object key;
  key.type = cscript_object_type_string;
  key.value.s = *name;
  cscript_map** map_it = cscript_vector_begin(&ctxt->environment, cscript_map*);
  cscript_map** map_it_end = cscript_vector_end(&ctxt->environment, cscript_map*);
  cscript_map** map_rit = map_it_end - 1;
  cscript_map** map_rit_end = map_it - 1;
  for (; map_rit != map_rit_end; --map_rit)
    {
    cscript_object* entry_object = cscript_map_get(ctxt, *map_rit, &key);
    if (entry_object != NULL)
      {
      entry->type = entry_object->type - 1;
      entry->position = entry_object->value.fx;
      return 1;
      }
    }
  return 0;
  }

void cscript_environment_update(cscript_context* ctxt, cscript_string* name, cscript_environment_entry entry)
  {  
  cscript_object key;
  key.type = cscript_object_type_string;
  key.value.s = *name;
  cscript_map** map_it = cscript_vector_begin(&ctxt->environment, cscript_map*);
  cscript_map** map_it_end = cscript_vector_end(&ctxt->environment, cscript_map*);
  cscript_map** map_rit = map_it_end - 1;
  cscript_map** map_rit_end = map_it - 1;
  for (; map_rit != map_rit_end; --map_rit)
    {
    cscript_object* entry_object = cscript_map_get(ctxt, *map_rit, &key);
    if (entry_object != NULL)
      {
      entry_object->type = entry.type + 1;
      entry_object->value.fx = entry.position;
      }
    }
  }

void cscript_environment_push_child(cscript_context* ctxt)
  {
  cscript_map* child_env = cscript_map_new(ctxt, 0, 3);
  cscript_vector_push_back(ctxt, &ctxt->environment, child_env, cscript_map*);
  }

void cscript_environment_pop_child(cscript_context* ctxt)
  {
  cscript_assert(ctxt->environment.vector_size > 1); // don't pop the root
  cscript_map** child_map = cscript_vector_back(&ctxt->environment, cscript_map*);
  cscript_map_keys_free(ctxt, *child_map);  
  cscript_map_free(ctxt, *child_map);
  cscript_vector_pop_back(&ctxt->environment);
  }

cscript_memsize cscript_environment_base_size(cscript_context* ctxt)
  {
  cscript_map** parent_map = cscript_vector_at(&ctxt->environment, 0, cscript_map*);
  return node_size((*parent_map));
  }

int cscript_environment_base_at(cscript_environment_entry* entry, cscript_string* name, cscript_context* ctxt, cscript_memsize pos)
  {
  cscript_map** parent_map = cscript_vector_at(&ctxt->environment, 0, cscript_map*);
  cscript_assert(pos < cast(cscript_memsize, node_size((*parent_map))));
  if (cscript_object_get_type(&(*parent_map)->node[pos].key) == cscript_object_type_string) // valid node
    {
    entry->type = (*parent_map)->node[pos].value.type - 1;
    entry->position = (*parent_map)->node[pos].value.value.fx;
    *name = (*parent_map)->node[pos].key.value.s;
    return 1;
    }  
  return 0;
  }

void cscript_show_environment(cscript_context* ctxt, cscript_string* s)
  {
  cscript_string_append_cstr(ctxt, s, "CSCRIPT ENVIRONMENT:\n");
  cscript_map** map_it = cscript_vector_begin(&ctxt->environment, cscript_map*);
  cscript_map** map_it_end = cscript_vector_end(&ctxt->environment, cscript_map*);
  for (; map_it != map_it_end; ++map_it)
    {
    cscript_map* m = *map_it;
    cscript_memsize size = node_size(m);
    for (cscript_memsize i = 0; i < size; ++i)
      {
      if (cscript_object_get_type(&m->node[i].key) == cscript_object_type_string)
        {
        cscript_string_append(ctxt, s, &m->node[i].key.value.s);
        cscript_string_append_cstr(ctxt, s, ": ");
        cscript_environment_entry entry;
        entry.type = m->node[i].value.type - 1;
        entry.position = m->node[i].value.value.fx;
        cscript_string tmp;
        if (entry.type == CSCRIPT_ENV_TYPE_STACK)
          {
          cscript_object* local = cscript_vector_at(&ctxt->stack, entry.position, cscript_object);
          tmp = cscript_object_to_string(ctxt, local, 0);
          }
        else
          {
          cscript_string tmp2 = cscript_object_to_string(ctxt, &m->node[i].value, 0);
          cscript_string_append(ctxt, s, &tmp2);
          cscript_string_destroy(ctxt, &tmp2);
          cscript_string_append_cstr(ctxt, s, ": ");
          cscript_object* global = cscript_vector_at(&ctxt->globals, entry.position, cscript_object);
          tmp = cscript_object_to_string(ctxt, global, 0);
          }
        cscript_string_append(ctxt, s, &tmp);

        cscript_string_append_cstr(ctxt, s, "\n");
        cscript_string_destroy(ctxt, &tmp);
        }      
      }
    }  
  }

cscript_object* cscript_environment_find_key_given_position(cscript_context* ctxt, cscript_fixnum global_position)
  {
  cscript_assert(ctxt->environment.vector_size > 0);
  cscript_map* base_map = *cscript_vector_begin(&ctxt->environment, cscript_map*);
  cscript_memsize size = node_size(base_map);
  for (cscript_memsize i = 0; i < size; ++i)
    {
    if (cscript_object_get_type(&base_map->node[i].key) == cscript_object_type_string)
      {
      if ((base_map->node[i].value.type == CSCRIPT_ENV_TYPE_GLOBAL + 1) && (base_map->node[i].value.value.fx == global_position))
        {
        return &base_map->node[i].key;
        }
      }
    }
  return NULL;
  }