#include "map.h"
#include "memory.h"
#include "limits.h"
#include "error.h"
#include "context.h"

#include <string.h>

#define hashmod(m,n) (get_node(m, ((n) % ((node_size(m)-1)|1))))

#define fxnumints cast(cscript_memsize, sizeof(cscript_fixnum)/sizeof(cscript_memsize))
#define flnumints cast(cscript_memsize, sizeof(cscript_flonum)/sizeof(cscript_memsize))

#define cscript_max_bits (cscript_mem_bits - 2)
#define toobig(x) ((((x)-1) >> cscript_max_bits) != 0)
#define twoto(x) ((cscript_memsize)1<<(x))

static void cscript_set_nil(cscript_object* obj)
  {
  obj->type = cscript_object_type_undefined;
  }

static void cscript_create_array_vector(cscript_context* ctxt, cscript_map* map, cscript_memsize array_size)
  {
  map->array = cscript_newvector(ctxt, array_size, cscript_object);
  for (cscript_memsize i = map->array_size; i < array_size; ++i)
    cscript_set_nil(&map->array[i]);
  map->array_size = array_size;
  }

static void cscript_create_node_vector(cscript_context* ctxt, cscript_map* map, cscript_memsize log_node_size)
  {
  cscript_memsize size = twoto(log_node_size);
  if (log_node_size > cscript_max_bits)
    cscript_throw(ctxt, CSCRIPT_ERROR_MEMORY);
  if (log_node_size == 0)
    {
    map->node = ctxt->global->dummy_node;
    cscript_assert(cscript_object_get_type(get_key(map->node)) == cscript_object_type_undefined);
    cscript_assert(cscript_object_get_type(get_value(map->node)) == cscript_object_type_undefined);
    cscript_assert(map->node->next == NULL);
    }
  else
    {
    map->node = cscript_newvector(ctxt, size, cscript_map_node);
    for (cscript_memsize i = 0; i < size; ++i)
      {
      map->node[i].next = NULL;
      cscript_set_nil(get_key(get_node(map, i)));
      cscript_set_nil(get_value(get_node(map, i)));
      }
    }
  map->log_node_size = log_node_size;
  map->first_free = get_node(map, size - 1);
  }

static void computesizes(cscript_memsize nums[], cscript_memsize ntotal, cscript_memsize* narray, cscript_memsize* nhash)
  {
  cscript_memsize i;
  cscript_memsize a = nums[0];  /* number of elements smaller than 2^i */
  cscript_memsize na = a;  /* number of elements to go to array part */
  cscript_memsize n = (na == 0) ? cscript_mem_invalid_size : 0;  /* (log of) optimal size for array part */
  for (i = 1; (a < *narray) && (*narray >= twoto(i - 1)); ++i)
    {
    if (nums[i] > 0)
      {
      a += nums[i];
      if (a >= twoto(i - 1))
        {  /* more than half elements in use? */
        n = i;
        na = a;
        }
      }
    }
  cscript_assert(na <= *narray && *narray <= ntotal);
  *nhash = ntotal - na;
  *narray = (n == cscript_mem_invalid_size) ? 0 : twoto(n);
  cscript_assert(na <= *narray && na >= *narray / 2);
  }

static cscript_memsize arrayindex(const cscript_object* key)
  {
  if (cscript_object_get_type(key) == cscript_object_type_fixnum)
    {
    cscript_memsize index = (cscript_memsize)key->value.fx;
    if (cast(cscript_fixnum, index) == key->value.fx && !toobig(index))
      return index;
    }
  return cscript_mem_invalid_size;  /* `key' did not match some condition */
  }


static void numuse(const cscript_map* t, cscript_memsize* narray, cscript_memsize* nhash)
  {
  cscript_memsize nums[cscript_max_bits + 1];
  cscript_memsize i, lg;
  cscript_memsize totaluse = 0;
  /* count elements in array part */
  for (i = 0, lg = 0; lg <= cscript_max_bits; ++lg)
    {  /* for each slice [2^(lg-1) to 2^lg) */
    cscript_memsize ttlg = twoto(lg);  /* 2^lg */
    if (ttlg > t->array_size)
      {
      ttlg = t->array_size;
      if (i >= ttlg)
        break;
      }
    nums[lg] = 0;
    for (; i < ttlg; ++i)
      {
      if (t->array[i].type != cscript_object_type_undefined)
        {
        ++nums[lg];
        ++totaluse;
        }
      }
    }
  for (; lg <= cscript_max_bits; ++lg)
    nums[lg] = 0;  /* reset other counts */
  *narray = totaluse;  /* all previous uses were in array part */
  /* count elements in hash part */
  i = node_size(t);
  while (i--)
    {
    cscript_map_node* n = &t->node[i];
    if (cscript_object_get_type(get_value(n)) != cscript_object_type_undefined)
      {
      cscript_memsize k = arrayindex(get_key(n));
      if (k != cscript_mem_invalid_size)
        {  /* is `key' an appropriate array index? */
        ++nums[cscript_log2(k - 1) + 1];  /* count as such */
        ++(*narray);
        }
      ++totaluse;
      }
    }
  computesizes(nums, totaluse, narray, nhash);
  }

static void resize(cscript_context* ctxt, cscript_map* t, cscript_memsize nasize, int nhsize)
  {
  cscript_memsize i;
  cscript_memsize oldasize = t->array_size;
  cscript_memsize oldhsize = t->log_node_size;
  cscript_map_node* nold;
  cscript_map_node temp[1];
  if (oldhsize)
    nold = t->node;  /* save old hash ... */
  else
    {  /* old hash is `dummynode' */
    cscript_assert(t->node == ctxt->global->dummy_node);
    temp[0] = t->node[0];  /* copy it to `temp' */
    nold = temp;
    get_key(ctxt->global->dummy_node)->type = cscript_object_type_undefined; /* restate invariant */
    get_value(ctxt->global->dummy_node)->type = cscript_object_type_undefined;
    cscript_assert(ctxt->global->dummy_node->next == NULL);
    }
  if (nasize > oldasize)  /* array part must grow? */
    cscript_create_array_vector(ctxt, t, nasize);
  /* create new hash part with appropriate size */
  cscript_create_node_vector(ctxt, t, nhsize);
  /* re-insert elements */
  if (nasize < oldasize)
    {  /* array part must shrink? */
    t->array_size = nasize;
    /* re-insert elements from vanishing slice */
    for (i = nasize; i < oldasize; ++i)
      {
      if (cscript_object_get_type(&t->array[i]) != cscript_object_type_undefined)
        {
        cscript_object* obj = cscript_map_insert_indexed(ctxt, t, i + 1);
        cscript_set_object(obj, &t->array[i]);
        }
      }
    /* shrink array */
    cscript_reallocvector(ctxt, t->array, oldasize, nasize, cscript_object);
    }
  /* re-insert elements in hash part */
  for (i = twoto(oldhsize); i >= 1; --i)
    {
    cscript_map_node* old = nold + i - 1;
    if (cscript_object_get_type(get_value(old)) != cscript_object_type_undefined)
      {
      cscript_object* obj = cscript_map_insert(ctxt, t, get_key(old));
      cscript_set_object(obj, get_value(old));
      }
    }
  if (oldhsize)
    cscript_freevector(ctxt, nold, twoto(oldhsize), cscript_map_node); /* free old array */
  }


static void cscript_rehash(cscript_context* ctxt, cscript_map* map)
  {
  cscript_memsize nasize, nhsize;
  numuse(map, &nasize, &nhsize);  /* compute new sizes for array and hash parts */
  resize(ctxt, map, nasize, cscript_log2(nhsize) + 1);
  }


cscript_map* cscript_map_new(cscript_context* ctxt, cscript_memsize array_size, cscript_memsize log_node_size)
  {
  cscript_map* m = cscript_new(ctxt, cscript_map);
  m->array = NULL;
  m->node = NULL;
  m->array_size = 0;
  m->log_node_size = 0;
  cscript_create_array_vector(ctxt, m, array_size);
  cscript_create_node_vector(ctxt, m, log_node_size);
  return m;
  }

void cscript_map_keys_free(cscript_context* ctxt, cscript_map* map)
  {
  cscript_memsize size = node_size(map);
  for (cscript_memsize i = 0; i < size; ++i)
    {
    if (cscript_object_get_type(&map->node[i].key) == cscript_object_type_string)
      cscript_string_destroy(ctxt, &(map->node[i].key.value.s));
    }
  }

void cscript_map_values_free(cscript_context* ctxt, cscript_map* map)
  {
  cscript_memsize size = node_size(map);
  for (cscript_memsize i = 0; i < size; ++i)
    {
    if (cscript_object_get_type(&map->node[i].value) == cscript_object_type_string)
      cscript_string_destroy(ctxt, &(map->node[i].value.value.s));    
    }
  }

void cscript_map_free(cscript_context* ctxt, cscript_map* map)
  {
  cscript_freevector(ctxt, map->node, node_size(map), cscript_map_node);
  cscript_freevector(ctxt, map->array, map->array_size, cscript_object);
  cscript_delete(ctxt, map);
  }

static cscript_map_node* cscript_hash_fixnum(const cscript_map* m, cscript_fixnum n)
  {
  cscript_memsize a[fxnumints];
  n += 1;
  memcpy(a, &n, sizeof(a));
  for (int i = 0; i < fxnumints; ++i)
    a[0] += a[i];
  return hashmod(m, a[0]);
  }

static cscript_map_node* cscript_hash_flonum(const cscript_map* m, cscript_flonum n)
  {
  cscript_memsize a[flnumints];
  n += 1;
  memcpy(a, &n, sizeof(a));
  for (int i = 0; i < flnumints; ++i)
    a[0] += a[i];
  return hashmod(m, a[0]);
  }

static cscript_map_node* cscript_hash_pointer(const cscript_map* m, void* ptr)
  {
  return hashmod(m, (uintptr_t)ptr);
  }

static cscript_memsize hash_string(const char* str)
  {
  int l = cast(int, strlen(str));
  cscript_memsize h = cast(cscript_memsize, l);
  int step = (l >> 5) + 1; // if string is too long, don't hash all its chars
  for (int i = l; i >= step; i -= step)
    h = h ^ ((h << 5) + (h >> 2) + (unsigned char)(str[i - 1]));
  return h;
  }

static cscript_map_node* cscript_hash_string(const cscript_map* m, const char* str)
  {
  cscript_memsize h = hash_string(str);
  cscript_assert((node_size(m) & (node_size(m) - 1)) == 0);
  return get_node(m, h & (node_size(m) - 1));
  }

static cscript_map_node* cscript_main_position(const cscript_map* m, const cscript_object* key)
  {
  switch (cscript_object_get_type(key))
    {
   
    case cscript_object_type_undefined:
      return cscript_hash_fixnum(m, cscript_object_get_type(key));   
    case cscript_object_type_fixnum:
      return cscript_hash_fixnum(m, key->value.fx);
    case cscript_object_type_flonum:
      return cscript_hash_flonum(m, key->value.fl);  
    case cscript_object_type_string:
      return cscript_hash_string(m, key->value.s.string_ptr);
    default:
      return cscript_hash_pointer(m, key->value.v.vector_ptr);
    }
  }

cscript_object* cscript_map_get_indexed(cscript_map* map, cscript_memsize index)
  {
  if (index < map->array_size)
    return &map->array[index];
  else
    {
    cscript_fixnum nr = cast(cscript_fixnum, index);
    cscript_map_node* n = cscript_hash_fixnum(map, nr);
    do
      {
      if (cscript_object_get_type(get_key(n)) == cscript_object_type_fixnum && get_key(n)->value.fx == nr)
        {
        return get_value(n);
        }
      else
        n = n->next;
      } while (n);
      return NULL;
    }
  }

static cscript_object* cscript_map_get_any(cscript_context* ctxt, cscript_map* map, const cscript_object* key)
  {
  if (cscript_object_get_type(key) == cscript_object_type_undefined)
    return NULL;
  else
    {
    cscript_map_node* n = cscript_main_position(map, key);
    do
      {
      if (cscript_objects_equal(ctxt, get_key(n), key))
        return get_value(n);
      else
        n = n->next;
      } while (n);
      return NULL;
    }
  }

cscript_object* cscript_map_get_string(cscript_map* map, const char* str)
  {
  cscript_map_node* n = cscript_hash_string(map, str);
  do
    {
    if ((cscript_object_get_type(get_key(n)) == cscript_object_type_string) && (strcmp(get_key(n)->value.s.string_ptr, str) == 0))
      return get_value(n);
    else
      n = n->next;
    } while (n);
    return NULL;
  }

cscript_object* cscript_map_get(cscript_context* ctxt, cscript_map* map, const cscript_object* key)
  {
  switch (cscript_object_get_type(key))
    {
    case cscript_object_type_string:
      return cscript_map_get_string(map, key->value.s.string_ptr);
    case cscript_object_type_fixnum:
    {
    cscript_memsize index = (cscript_memsize)key->value.fx;
    if (cast(cscript_fixnum, index) == key->value.fx)
      {
      return cscript_map_get_indexed(map, index); // use specialized version, else go to default
      }
    }
    default:
      return cscript_map_get_any(ctxt, map, key);
    }
  }

static cscript_object* new_key(cscript_context* ctxt, cscript_map* map, const cscript_object* key)
  {
  cscript_map_node* main = cscript_main_position(map, key);
  if (cscript_object_get_type(get_value(main)) != cscript_object_type_undefined) // main position is not free
    {
    cscript_map_node* other = cscript_main_position(map, get_key(main)); // main position of colliding node
    cscript_map_node* n = map->first_free; // get a free place
    if (other != main) // if colliding node is out of its main position
      {
      //move colliding node into free position
      while (other->next != main)
        other = other->next;
      other->next = n; //redo the chain with n in place of main
      *n = *main; // copy colliding node into ree position
      main->next = NULL;
      get_value(main)->type = cscript_object_type_undefined;
      }
    else // colliding node is in its own main position
      {
      // new node will go into free position
      n->next = main->next; // chain
      main->next = n;
      main = n;
      }
    }
  cscript_set_object(get_key(main), key);
  cscript_assert(cscript_object_get_type(get_value(main)) == cscript_object_type_undefined);
  for (;;) // set first_free correctly
    {
    if (cscript_object_get_type(get_key(map->first_free)) == cscript_object_type_undefined) // map still has a free place
      return get_value(main);
    else if (map->first_free == map->node) //cannot decrement from here
      break;
    else
      --(map->first_free);
    }
  // there are no free places anymore.
  get_value(main)->type = cscript_object_type_fixnum;
  cscript_rehash(ctxt, map); // grow map
  cscript_object* val = cast(cscript_object*, cscript_map_get(ctxt, map, key));
  cscript_assert(cscript_object_get_type(val) == cscript_object_type_fixnum);
  val->type = cscript_object_type_undefined;
  return val;
  }

cscript_object* cscript_map_insert_indexed(cscript_context* ctxt, cscript_map* map, cscript_memsize index)
  {
  cscript_object* p = cscript_map_get_indexed(map, index);
  if (p != NULL)
    return p;
  else
    {
    cscript_object k;
    k.type = cscript_object_type_fixnum;
    k.value.fx = cast(cscript_fixnum, index);
    return new_key(ctxt, map, &k);
    }
  }

cscript_object* cscript_map_insert(cscript_context* ctxt, cscript_map* map, const cscript_object* key)
  {
  cscript_object* p = cscript_map_get(ctxt, map, key);
  if (p != NULL)
    return p;
  else
    {
    if (cscript_object_get_type(key) == cscript_object_type_undefined)
      cscript_throw(ctxt, CSCRIPT_ERROR_INVALID_ARGUMENT);
    return new_key(ctxt, map, key);
    }
  }

cscript_object* cscript_map_insert_string(cscript_context* ctxt, cscript_map* map, const char* str)
  {
  cscript_object* p = cscript_map_get_string(map, str);
  if (p != NULL)
    return p;
  else
    {
    cscript_object key = make_cscript_object_string(ctxt, str);
    cscript_object* obj = new_key(ctxt, map, &key);
    return obj;
    }
  }
