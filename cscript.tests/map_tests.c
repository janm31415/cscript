#include "map_tests.h"
#include "test_assert.h"

#include "cscript/context.h"
#include "cscript/map.h"

static void test_map_construction()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_map* m = cscript_map_new(ctxt, 10, 4);

  TEST_EQ_INT(10, m->array_size);
  TEST_EQ_INT(4, m->log_node_size);

  cscript_map_free(ctxt, m);
  cscript_close(ctxt);
  }

static void test_map()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_map* m = cscript_map_new(ctxt, 10, 4);

  cscript_object* obj = cscript_map_get_indexed(m, 0);
  TEST_EQ_INT(cscript_object_type_undefined, obj->type);

  obj = cscript_map_insert_indexed(ctxt, m, 0);
  TEST_EQ_INT(cscript_object_type_undefined, obj->type);
  TEST_EQ_INT(10, m->array_size);
  TEST_EQ_INT(4, m->log_node_size);
  obj->type = cscript_object_type_flonum;
  obj->value.fl = 3.14;

  obj = cscript_map_get_indexed(m, 0);
  TEST_EQ_INT(cscript_object_type_flonum, obj->type);
  TEST_EQ_DOUBLE(3.14, obj->value.fl);

  cscript_map_free(ctxt, m);
  cscript_close(ctxt);
  }

static void test_map_2()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_map* m = cscript_map_new(ctxt, 10, 4);

  cscript_object* obj = cscript_map_get_indexed(m, 12);
  TEST_EQ_INT(0, (uintptr_t)obj);

  obj = cscript_map_insert_indexed(ctxt, m, 12);
  TEST_EQ_INT(cscript_object_type_undefined, obj->type);
  TEST_EQ_INT(10, m->array_size);
  TEST_EQ_INT(4, m->log_node_size);
  obj->type = cscript_object_type_flonum;
  obj->value.fl = 3.14;

  obj = cscript_map_get_indexed(m, 12);
  TEST_EQ_INT(cscript_object_type_flonum, obj->type);
  TEST_EQ_DOUBLE(3.14, obj->value.fl);

  cscript_map_free(ctxt, m);
  cscript_close(ctxt);
  }

static void test_map_3()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_map* m = cscript_map_new(ctxt, 10, 4);

  cscript_object* obj = cscript_map_get_indexed(m, 200000);
  TEST_EQ_INT(0, (uintptr_t)obj);

  obj = cscript_map_insert_indexed(ctxt, m, 200000);
  TEST_EQ_INT(cscript_object_type_undefined, obj->type);
  TEST_EQ_INT(10, m->array_size);
  TEST_EQ_INT(4, m->log_node_size);
  obj->type = cscript_object_type_flonum;
  obj->value.fl = 3.14;

  obj = cscript_map_get_indexed(m, 200000);
  TEST_EQ_INT(cscript_object_type_flonum, obj->type);
  TEST_EQ_DOUBLE(3.14, obj->value.fl);

  cscript_map_free(ctxt, m);
  cscript_close(ctxt);
  }

static void test_map_4()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_map* m = cscript_map_new(ctxt, 10, 1);

  cscript_object* obj20 = cscript_map_insert_indexed(ctxt, m, 20);
  TEST_EQ_INT(cscript_object_type_undefined, obj20->type);
  TEST_EQ_INT(10, m->array_size);
  TEST_EQ_INT(1, m->log_node_size);
  obj20->type = cscript_object_type_fixnum;
  obj20->value.fx = 20;
  
  cscript_object* obj21 = cscript_map_insert_indexed(ctxt, m, 21);
  TEST_EQ_INT(cscript_object_type_undefined, obj21->type);
  TEST_EQ_INT(0, m->array_size);
  TEST_EQ_INT(2, m->log_node_size);
  obj21->type = cscript_object_type_fixnum;
  obj21->value.fx = 21;

  cscript_object* obj22 = cscript_map_insert_indexed(ctxt, m, 22);
  TEST_EQ_INT(cscript_object_type_undefined, obj22->type);
  TEST_EQ_INT(0, m->array_size);
  TEST_EQ_INT(2, m->log_node_size);
  obj22->type = cscript_object_type_fixnum;
  obj22->value.fx = 22;

  cscript_object* obj23 = cscript_map_insert_indexed(ctxt, m, 23);
  TEST_EQ_INT(cscript_object_type_undefined, obj23->type);
  TEST_EQ_INT(0, m->array_size);
  TEST_EQ_INT(3, m->log_node_size);
  obj23->type = cscript_object_type_fixnum;
  obj23->value.fx = 23;

  TEST_EQ_INT(cscript_object_type_fixnum, cscript_map_get_indexed(m, 20)->type);
  TEST_EQ_INT(cscript_object_type_fixnum, cscript_map_get_indexed(m, 21)->type);
  TEST_EQ_INT(cscript_object_type_fixnum, cscript_map_get_indexed(m, 22)->type);
  TEST_EQ_INT(cscript_object_type_fixnum, cscript_map_get_indexed(m, 23)->type);
  TEST_EQ_INT(20, cscript_map_get_indexed(m, 20)->value.fx);
  TEST_EQ_INT(21, cscript_map_get_indexed(m, 21)->value.fx);
  TEST_EQ_INT(22, cscript_map_get_indexed(m, 22)->value.fx);
  TEST_EQ_INT(23, cscript_map_get_indexed(m, 23)->value.fx);
  TEST_EQ_INT(0, m->array_size);
  TEST_EQ_INT(3, m->log_node_size);

  cscript_map_free(ctxt, m);
  cscript_close(ctxt);
  }

static void test_map_with_string_keys()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_map* m = cscript_map_new(ctxt, 0, 1);

  cscript_object* obj_alpha = cscript_map_insert_string(ctxt, m, "alpha");
  obj_alpha->type = cscript_object_type_fixnum;
  obj_alpha->value.fx = 2000;

  cscript_object* obj_beta = cscript_map_insert_string(ctxt, m, "beta");
  obj_beta->type = cscript_object_type_fixnum;
  obj_beta->value.fx = 2001;

  cscript_object* obj_gamma = cscript_map_insert_string(ctxt, m, "gamma");
  obj_gamma->type = cscript_object_type_fixnum;
  obj_gamma->value.fx = 2002;

  cscript_object* obj_delta = cscript_map_insert_string(ctxt, m, "delta");
  obj_delta->type = cscript_object_type_fixnum;
  obj_delta->value.fx = 2003;

  cscript_object* obj_find_alpha = cscript_map_get_string(m, "alpha");
  TEST_EQ_INT(cscript_object_type_fixnum, obj_find_alpha->type);
  TEST_EQ_INT(2000, obj_find_alpha->value.fx);

  cscript_object* obj_find_beta = cscript_map_get_string(m, "beta");
  TEST_EQ_INT(cscript_object_type_fixnum, obj_find_beta->type);
  TEST_EQ_INT(2001, obj_find_beta->value.fx);

  cscript_object* obj_find_gamma = cscript_map_get_string(m, "gamma");
  TEST_EQ_INT(cscript_object_type_fixnum, obj_find_gamma->type);
  TEST_EQ_INT(2002, obj_find_gamma->value.fx);

  cscript_object* obj_find_delta = cscript_map_get_string(m, "delta");
  TEST_EQ_INT(cscript_object_type_fixnum, obj_find_delta->type);
  TEST_EQ_INT(2003, obj_find_delta->value.fx);

  cscript_map_keys_free(ctxt, m);
  cscript_map_free(ctxt, m);
  cscript_close(ctxt);
  }

void run_all_map_tests()
  {
  test_map_construction();
  test_map();
  test_map_2();
  test_map_3();
  test_map_4();
  test_map_with_string_keys();
  }
