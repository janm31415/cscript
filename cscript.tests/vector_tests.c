#include "vector_tests.h"
#include "test_assert.h"

#include "cscript/cscript.h"
#include "cscript/vector.h"
#include "cscript/context.h"

static void test_construction_destruction_1()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector v;
  cscript_vector_init_with_size(ctxt, &v, 5, double);
  TEST_EQ_INT(5, v.vector_size);
  TEST_EQ_INT(5, v.vector_capacity);
  TEST_EQ_INT(sizeof(double), v.element_size);
  cscript_vector_destroy(ctxt, &v);
  cscript_close(ctxt);
  }

static void test_construction_destruction_2()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector v;
  cscript_vector_init(ctxt, &v, double);
  TEST_EQ_INT(0, v.vector_size);
  TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR, v.vector_capacity);
  TEST_EQ_INT(sizeof(double), v.element_size);
  cscript_vector_destroy(ctxt, &v);
  cscript_close(ctxt);
  }

static void test_access()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector v;
  cscript_vector_init_with_size(ctxt, &v, 5, double);
  *cscript_vector_at(&v, 0, double) = 1.0;
  *cscript_vector_at(&v, 1, double) = 2.0;
  *cscript_vector_at(&v, 2, double) = 3.0;
  *cscript_vector_at(&v, 3, double) = 4.0;
  *cscript_vector_at(&v, 4, double) = 5.0;
  TEST_EQ_DOUBLE(1.0, *cscript_vector_at(&v, 0, double));
  TEST_EQ_DOUBLE(2.0, *cscript_vector_at(&v, 1, double));
  TEST_EQ_DOUBLE(3.0, *cscript_vector_at(&v, 2, double));
  TEST_EQ_DOUBLE(4.0, *cscript_vector_at(&v, 3, double));
  TEST_EQ_DOUBLE(5.0, *cscript_vector_at(&v, 4, double));
  cscript_vector_destroy(ctxt, &v);
  cscript_close(ctxt);
  }

static void test_pop_back()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector v;
  cscript_vector_init_with_size(ctxt, &v, 5, double);
  TEST_EQ_INT(5, v.vector_size);
  TEST_EQ_INT(5, v.vector_capacity);
  cscript_vector_pop_back(&v);
  TEST_EQ_INT(4, v.vector_size);
  TEST_EQ_INT(5, v.vector_capacity);
  cscript_vector_pop_back(&v);
  TEST_EQ_INT(3, v.vector_size);
  TEST_EQ_INT(5, v.vector_capacity);
  cscript_vector_pop_back(&v);
  TEST_EQ_INT(2, v.vector_size);
  TEST_EQ_INT(5, v.vector_capacity);
  cscript_vector_pop_back(&v);
  TEST_EQ_INT(1, v.vector_size);
  TEST_EQ_INT(5, v.vector_capacity);
  cscript_vector_pop_back(&v);
  TEST_EQ_INT(0, v.vector_size);
  TEST_EQ_INT(5, v.vector_capacity);
  cscript_vector_pop_back(&v);
  TEST_EQ_INT(0, v.vector_size);
  TEST_EQ_INT(5, v.vector_capacity);
  cscript_vector_destroy(ctxt, &v);
  cscript_close(ctxt);
  }

static void test_push_back()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector v;
  cscript_vector_init(ctxt, &v, double);
  TEST_EQ_INT(0, v.vector_size);
  TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR, v.vector_capacity);
  cscript_vector_push_back(ctxt, &v, 7.0, double);
  TEST_EQ_INT(1, v.vector_size);
  TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR, v.vector_capacity);
  cscript_vector_push_back(ctxt, &v, 9.0, double);
  TEST_EQ_INT(2, v.vector_size);
  TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR < 2 ? 2 : CSCRIPT_MINSIZEVECTOR, v.vector_capacity);
  TEST_EQ_DOUBLE(7.0, *cscript_vector_at(&v, 0, double));
  TEST_EQ_DOUBLE(9.0, *cscript_vector_at(&v, 1, double));
  cscript_vector_destroy(ctxt, &v);
  cscript_close(ctxt);
  }

static void test_push_front()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector v;
  cscript_vector_init(ctxt, &v, double);
  TEST_EQ_INT(0, v.vector_size);
  TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR, v.vector_capacity);
  cscript_vector_push_front(ctxt, &v, 7.0, double);
  TEST_EQ_INT(1, v.vector_size);
  TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR, v.vector_capacity);
  cscript_vector_push_front(ctxt, &v, 9.0, double);
  TEST_EQ_INT(2, v.vector_size);
  TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR < 2 ? 2 : CSCRIPT_MINSIZEVECTOR, v.vector_capacity);
  TEST_EQ_DOUBLE(9.0, *cscript_vector_at(&v, 0, double));
  TEST_EQ_DOUBLE(7.0, *cscript_vector_at(&v, 1, double));
  cscript_vector_destroy(ctxt, &v);
  cscript_close(ctxt);
  }

static void test_push_back_2()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector v;
  cscript_vector_init_with_size(ctxt, &v, 0, double);
  TEST_EQ_INT(0, v.vector_size);
  TEST_EQ_INT(0, v.vector_capacity);

  for (int i = 0; i < CSCRIPT_MINSIZEVECTOR; ++i)
    {
    cscript_vector_push_back(ctxt, &v, 7.0 + i * 2, double);
    TEST_EQ_INT(i + 1, v.vector_size);
    TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR, v.vector_capacity);
    }

  for (int i = 0; i < CSCRIPT_MINSIZEVECTOR; ++i)
    {
    TEST_EQ_DOUBLE(7.0 + i*2, *cscript_vector_at(&v, i, double));
    }
    
  cscript_vector_push_back(ctxt, &v, 15.0, double);
  TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR+1, v.vector_size);
  TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR*2, v.vector_capacity);  
  cscript_vector_destroy(ctxt, &v);
  cscript_close(ctxt);
  }

static void test_iterator()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector v;
  cscript_vector_init(ctxt, &v, double);
  TEST_EQ_INT(0, v.vector_size);
  TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR, v.vector_capacity);
  cscript_vector_push_back(ctxt, &v, 7.0, double);
  TEST_EQ_INT(1, v.vector_size);
  TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR, v.vector_capacity);
  cscript_vector_push_back(ctxt, &v, 9.0, double);
  TEST_EQ_INT(2, v.vector_size);
  TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR < 2 ? 2 : CSCRIPT_MINSIZEVECTOR, v.vector_capacity);
  double* it = cscript_vector_begin(&v, double);
  double* it_end = cscript_vector_end(&v, double);
  int counter = 0;
  for (; it != it_end; ++it, ++counter)
    {
    if (counter == 0)
      {
      TEST_EQ_DOUBLE(7.0, *it);
      }
    if (counter == 1)
      {
      TEST_EQ_DOUBLE(9.0, *it);
      }
    }
  TEST_EQ_INT(2, counter);
  cscript_vector_destroy(ctxt, &v);
  cscript_close(ctxt);
  }

static void test_iterator_2()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector v;
  cscript_vector_init(ctxt, &v, double);
  TEST_EQ_INT(0, v.vector_size);
  TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR, v.vector_capacity);
  cscript_vector_push_back(ctxt, &v, 7.0, double);
  TEST_EQ_INT(1, v.vector_size);
  TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR, v.vector_capacity);
  cscript_vector_push_back(ctxt, &v, 9.0, double);
  TEST_EQ_INT(2, v.vector_size);
  TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR < 2 ? 2 : CSCRIPT_MINSIZEVECTOR, v.vector_capacity);
  double* it = cscript_vector_begin(&v, double);
  double* it_end = cscript_vector_end(&v, double);
  int counter = 0;
  for (; it != it_end; ++it, ++counter)
    {
    *it = (double)counter;
    }
  TEST_EQ_INT(2, counter);
  TEST_EQ_DOUBLE(0.0, *cscript_vector_at(&v, 0, double));
  TEST_EQ_DOUBLE(1.0, *cscript_vector_at(&v, 1, double));
  cscript_vector_destroy(ctxt, &v);
  cscript_close(ctxt);
  }

static void test_back()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector v;
  cscript_vector_init(ctxt, &v, double);
  TEST_EQ_INT(0, v.vector_size);
  TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR, v.vector_capacity);
  cscript_vector_push_back(ctxt, &v, 7.0, double);
  TEST_EQ_INT(1, v.vector_size);
  TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR, v.vector_capacity);
  cscript_vector_push_back(ctxt, &v, 9.0, double);
  TEST_EQ_INT(2, v.vector_size);
  TEST_EQ_INT(CSCRIPT_MINSIZEVECTOR < 2 ? 2 : CSCRIPT_MINSIZEVECTOR, v.vector_capacity);
  TEST_EQ_DOUBLE(9.0, *cscript_vector_back(&v, double));
  cscript_vector_destroy(ctxt, &v);
  cscript_close(ctxt);
  }

static void test_erase()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector v;
  cscript_vector_init_with_size(ctxt, &v, 5, double);
  *cscript_vector_at(&v, 0, double) = 1.0;
  *cscript_vector_at(&v, 1, double) = 2.0;
  *cscript_vector_at(&v, 2, double) = 3.0;
  *cscript_vector_at(&v, 3, double) = 4.0;
  *cscript_vector_at(&v, 4, double) = 5.0;
  double* it = cscript_vector_begin(&v, double);
  ++it;
  ++it;
  TEST_EQ_DOUBLE(3.0, *it);
  cscript_vector_erase(&v, &it, double);
  TEST_EQ_INT(4, v.vector_size);
  TEST_EQ_DOUBLE(1.0, *cscript_vector_at(&v, 0, double));
  TEST_EQ_DOUBLE(2.0, *cscript_vector_at(&v, 1, double));
  TEST_EQ_DOUBLE(4.0, *cscript_vector_at(&v, 2, double));
  TEST_EQ_DOUBLE(5.0, *cscript_vector_at(&v, 3, double));
  TEST_EQ_DOUBLE(4.0, *it);
  cscript_vector_erase(&v, &it, double);
  TEST_EQ_INT(3, v.vector_size);
  TEST_EQ_DOUBLE(1.0, *cscript_vector_at(&v, 0, double));
  TEST_EQ_DOUBLE(2.0, *cscript_vector_at(&v, 1, double));
  TEST_EQ_DOUBLE(5.0, *cscript_vector_at(&v, 2, double));
  TEST_EQ_DOUBLE(5.0, *it);
  cscript_vector_erase(&v, &it, double);
  TEST_EQ_INT(2, v.vector_size);
  TEST_EQ_DOUBLE(1.0, *cscript_vector_at(&v, 0, double));
  TEST_EQ_DOUBLE(2.0, *cscript_vector_at(&v, 1, double));
  double* it_end = cscript_vector_end(&v, double);
  TEST_EQ_INT((uintptr_t)it_end, (uintptr_t)it);
  cscript_vector_destroy(ctxt, &v);
  cscript_close(ctxt);
  }

static void test_erase_2()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector v;
  cscript_vector_init_with_size(ctxt, &v, 5, uint16_t);
  *cscript_vector_at(&v, 0, uint16_t) = 1;
  *cscript_vector_at(&v, 1, uint16_t) = 2;
  *cscript_vector_at(&v, 2, uint16_t) = 3;
  *cscript_vector_at(&v, 3, uint16_t) = 4;
  *cscript_vector_at(&v, 4, uint16_t) = 5;
  uint16_t* it = cscript_vector_begin(&v, uint16_t);
  ++it;
  ++it;
  TEST_EQ_DOUBLE(3, *it);
  cscript_vector_erase(&v, &it, uint16_t);
  TEST_EQ_INT(4, v.vector_size);
  TEST_EQ_INT(1, *cscript_vector_at(&v, 0, uint16_t));
  TEST_EQ_INT(2, *cscript_vector_at(&v, 1, uint16_t));
  TEST_EQ_INT(4, *cscript_vector_at(&v, 2, uint16_t));
  TEST_EQ_INT(5, *cscript_vector_at(&v, 3, uint16_t));
  TEST_EQ_INT(4, *it);
  cscript_vector_erase(&v, &it, uint16_t);
  TEST_EQ_INT(3, v.vector_size);
  TEST_EQ_INT(1, *cscript_vector_at(&v, 0, uint16_t));
  TEST_EQ_INT(2, *cscript_vector_at(&v, 1, uint16_t));
  TEST_EQ_INT(5, *cscript_vector_at(&v, 2, uint16_t));
  TEST_EQ_INT(5, *it);
  cscript_vector_erase(&v, &it, uint16_t);
  TEST_EQ_INT(2, v.vector_size);
  TEST_EQ_INT(1.0, *cscript_vector_at(&v, 0, uint16_t));
  TEST_EQ_INT(2.0, *cscript_vector_at(&v, 1, uint16_t));
  uint16_t* it_end = cscript_vector_end(&v, uint16_t);
  TEST_EQ_INT((uintptr_t)it_end, (uintptr_t)it);
  cscript_vector_destroy(ctxt, &v);
  cscript_close(ctxt);
  }

static void test_insert()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector v;
  cscript_vector_init_with_size(ctxt, &v, 5, double);
  *cscript_vector_at(&v, 0, double) = 1.0;
  *cscript_vector_at(&v, 1, double) = 2.0;
  *cscript_vector_at(&v, 2, double) = 3.0;
  *cscript_vector_at(&v, 3, double) = 4.0;
  *cscript_vector_at(&v, 4, double) = 5.0;
  double* it = cscript_vector_begin(&v, double);
  ++it;
  ++it;

  cscript_vector v2;
  cscript_vector_init_with_size(ctxt, &v2, 8, double);
  for (int i = 0; i < 8; ++i)
    *cscript_vector_at(&v2, i, double) = (double)(i)+10.0;

  double* it2 = cscript_vector_begin(&v2, double);
  double* it2_end = cscript_vector_end(&v2, double);

  cscript_vector_insert(ctxt, &v, &it, &it2, &it2_end, double);

  TEST_EQ_DOUBLE(1.0, *cscript_vector_at(&v, 0, double));
  TEST_EQ_DOUBLE(2.0, *cscript_vector_at(&v, 1, double));
  TEST_EQ_DOUBLE(10.0, *cscript_vector_at(&v, 2, double));
  TEST_EQ_DOUBLE(11.0, *cscript_vector_at(&v, 3, double));
  TEST_EQ_DOUBLE(12.0, *cscript_vector_at(&v, 4, double));
  TEST_EQ_DOUBLE(13.0, *cscript_vector_at(&v, 5, double));
  TEST_EQ_DOUBLE(14.0, *cscript_vector_at(&v, 6, double));
  TEST_EQ_DOUBLE(15.0, *cscript_vector_at(&v, 7, double));
  TEST_EQ_DOUBLE(16.0, *cscript_vector_at(&v, 8, double));
  TEST_EQ_DOUBLE(17.0, *cscript_vector_at(&v, 9, double));
  TEST_EQ_DOUBLE(3.0, *cscript_vector_at(&v, 10, double));
  TEST_EQ_DOUBLE(4.0, *cscript_vector_at(&v, 11, double));
  TEST_EQ_DOUBLE(5.0, *cscript_vector_at(&v, 12, double));

  TEST_EQ_DOUBLE(10.0, *it);

  cscript_vector_destroy(ctxt, &v);
  cscript_vector_destroy(ctxt, &v2);
  cscript_close(ctxt);
  }

static void test_insert_2()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector v;
  cscript_vector_init_with_size(ctxt, &v, 5, uint16_t);
  *cscript_vector_at(&v, 0, uint16_t) = 1;
  *cscript_vector_at(&v, 1, uint16_t) = 2;
  *cscript_vector_at(&v, 2, uint16_t) = 3;
  *cscript_vector_at(&v, 3, uint16_t) = 4;
  *cscript_vector_at(&v, 4, uint16_t) = 5;
  uint16_t* it = cscript_vector_begin(&v, uint16_t);
  ++it;
  ++it;

  cscript_vector v2;
  cscript_vector_init_with_size(ctxt, &v2, 8, uint16_t);
  for (int i = 0; i < 8; ++i)
    *cscript_vector_at(&v2, i, uint16_t) = (uint16_t)(i)+10;

  uint16_t* it2 = cscript_vector_begin(&v2, uint16_t);
  uint16_t* it2_end = cscript_vector_end(&v2, uint16_t);

  cscript_vector_insert(ctxt, &v, &it, &it2, &it2_end, uint16_t);

  TEST_EQ_INT(1, *cscript_vector_at(&v, 0, uint16_t));
  TEST_EQ_INT(2, *cscript_vector_at(&v, 1, uint16_t));
  TEST_EQ_INT(10, *cscript_vector_at(&v, 2, uint16_t));
  TEST_EQ_INT(11, *cscript_vector_at(&v, 3, uint16_t));
  TEST_EQ_INT(12, *cscript_vector_at(&v, 4, uint16_t));
  TEST_EQ_INT(13, *cscript_vector_at(&v, 5, uint16_t));
  TEST_EQ_INT(14, *cscript_vector_at(&v, 6, uint16_t));
  TEST_EQ_INT(15, *cscript_vector_at(&v, 7, uint16_t));
  TEST_EQ_INT(16, *cscript_vector_at(&v, 8, uint16_t));
  TEST_EQ_INT(17, *cscript_vector_at(&v, 9, uint16_t));
  TEST_EQ_INT(3, *cscript_vector_at(&v, 10, uint16_t));
  TEST_EQ_INT(4, *cscript_vector_at(&v, 11, uint16_t));
  TEST_EQ_INT(5, *cscript_vector_at(&v, 12, uint16_t));

  TEST_EQ_INT(10, *it);

  cscript_vector_destroy(ctxt, &v);
  cscript_vector_destroy(ctxt, &v2);
  cscript_close(ctxt);
  }

static void test_insert_element()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector v;
  cscript_vector_init_with_size(ctxt, &v, 5, uint16_t);
  *cscript_vector_at(&v, 0, uint16_t) = 1;
  *cscript_vector_at(&v, 1, uint16_t) = 2;
  *cscript_vector_at(&v, 2, uint16_t) = 3;
  *cscript_vector_at(&v, 3, uint16_t) = 4;
  *cscript_vector_at(&v, 4, uint16_t) = 5;
  uint16_t* it = cscript_vector_begin(&v, uint16_t);
  ++it;
  ++it;

  cscript_vector_insert_element(ctxt, &v, &it, 10, uint16_t);

  TEST_EQ_INT(1, *cscript_vector_at(&v, 0, uint16_t));
  TEST_EQ_INT(2, *cscript_vector_at(&v, 1, uint16_t));
  TEST_EQ_INT(10, *cscript_vector_at(&v, 2, uint16_t));
  TEST_EQ_INT(3, *cscript_vector_at(&v, 3, uint16_t));
  TEST_EQ_INT(4, *cscript_vector_at(&v, 4, uint16_t));
  TEST_EQ_INT(5, *cscript_vector_at(&v, 5, uint16_t));

  TEST_EQ_INT(10, *it);

  cscript_vector_destroy(ctxt, &v);
  cscript_close(ctxt);
  }


static void test_pop_front()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector v;
  cscript_vector_init_with_size(ctxt, &v, 5, double);
  *cscript_vector_at(&v, 0, double) = 1.0;
  *cscript_vector_at(&v, 1, double) = 2.0;
  *cscript_vector_at(&v, 2, double) = 3.0;
  *cscript_vector_at(&v, 3, double) = 4.0;
  *cscript_vector_at(&v, 4, double) = 5.0;
  cscript_vector_pop_front(&v);
  TEST_EQ_DOUBLE(2.0, *cscript_vector_at(&v, 0, double));
  TEST_EQ_DOUBLE(3.0, *cscript_vector_at(&v, 1, double));
  TEST_EQ_DOUBLE(4.0, *cscript_vector_at(&v, 2, double));
  TEST_EQ_DOUBLE(5.0, *cscript_vector_at(&v, 3, double));
  TEST_EQ_INT(4, v.vector_size);
  cscript_vector_pop_front(&v);
  TEST_EQ_DOUBLE(3.0, *cscript_vector_at(&v, 0, double));
  TEST_EQ_DOUBLE(4.0, *cscript_vector_at(&v, 1, double));
  TEST_EQ_DOUBLE(5.0, *cscript_vector_at(&v, 2, double));
  TEST_EQ_INT(3, v.vector_size);
  cscript_vector_pop_front(&v);
  TEST_EQ_DOUBLE(4.0, *cscript_vector_at(&v, 0, double));
  TEST_EQ_DOUBLE(5.0, *cscript_vector_at(&v, 1, double));
  TEST_EQ_INT(2, v.vector_size);
  cscript_vector_pop_front(&v);
  TEST_EQ_DOUBLE(5.0, *cscript_vector_at(&v, 0, double));
  TEST_EQ_INT(1, v.vector_size);
  cscript_vector_pop_front(&v);
  TEST_EQ_INT(0, v.vector_size);
  cscript_vector_pop_front(&v);
  TEST_EQ_INT(0, v.vector_size);
  cscript_vector_destroy(ctxt, &v);
  cscript_close(ctxt);
  }

static void test_erase_range()
  {
  cscript_context* ctxt = cscript_open(256);
  cscript_vector v;
  cscript_vector_init_with_size(ctxt, &v, 7, double);
  *cscript_vector_at(&v, 0, double) = 1.0;
  *cscript_vector_at(&v, 1, double) = 2.0;
  *cscript_vector_at(&v, 2, double) = 3.0;
  *cscript_vector_at(&v, 3, double) = 4.0;
  *cscript_vector_at(&v, 4, double) = 5.0;
  *cscript_vector_at(&v, 5, double) = 6.0;
  *cscript_vector_at(&v, 6, double) = 7.0;

  double* it = cscript_vector_begin(&v, double);
  //double* it_end = cscript_vector_end(&v, double);

  double* first = it + 2;
  double* last = it + 4;

  cscript_vector_erase_range(&v, &first, &last, double);

  TEST_EQ_DOUBLE(1.0, *cscript_vector_at(&v, 0, double));
  TEST_EQ_DOUBLE(2.0, *cscript_vector_at(&v, 1, double));
  TEST_EQ_DOUBLE(5.0, *cscript_vector_at(&v, 2, double));
  TEST_EQ_DOUBLE(6.0, *cscript_vector_at(&v, 3, double));
  TEST_EQ_DOUBLE(7.0, *cscript_vector_at(&v, 4, double));
  TEST_EQ_INT(5, v.vector_size);
  cscript_vector_destroy(ctxt, &v);
  cscript_close(ctxt);
  }

void run_all_vector_tests()
  {
  test_construction_destruction_1();
  test_construction_destruction_2();
  test_access();
  test_pop_back();
  test_push_back();
  test_push_back_2();
  test_iterator();
  test_iterator_2();
  test_back();
  test_erase();
  test_erase_2();
  test_insert();
  test_insert_2();
  test_insert_element();
  test_push_front();
  test_pop_front();
  test_erase_range();
  }
