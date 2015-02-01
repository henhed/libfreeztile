/* Tests for `class.c' functions.
   Copyright (C) 2013-2014 Henrik Hedelund.

   This file is part of libfreeztile.

   libfreeztile is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 3, or (at your option) any later
   version.

   libfreeztile is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License
   along with libfreeztile; see the file COPYING.  If not see
   <http://www.gnu.org/licenses/>.  */

#include <stdlib.h>
#include <time.h>
#include <check.h>
#include <errno.h>
#include "class.h"
#include "malloc.h"

/* Definition of a test class with some member variables.  */
typedef struct {
  class_t *__class;
  int_t a;
  int_t b;
  int_t *c;
} test_class_t;

typedef struct {
  class_t *__class;
} second_class_t;

/* Test class instance instantiated in `setup'.  */
test_class_t *test_instance = NULL;

/* Flag used to assert destructor invocation.  */
int_t test_destuctor_called = 0;

/* Test class constructor.  */
static ptr_t
test_constructor (ptr_t ptr, va_list *args)
{
  test_class_t *self = (test_class_t *) ptr;
  self->a = va_arg (*args, int_t);
  self->b = va_arg (*args, int_t);
  self->c = (int_t *) fz_malloc (sizeof (int_t));
  *self->c = va_arg (*args, int_t);
  return self;
}

/* Test class destructor.   */
static ptr_t
test_destructor (ptr_t ptr)
{
  test_class_t *self = (test_class_t *) ptr;
  fz_free (self->c);
  test_destuctor_called = 1;
  return ptr;
}

/* Test class length measurement implementation.  */
static size_t
test_length (ptr_t ptr)
{
  test_class_t *self = (test_class_t *) ptr;
  return self->a + self->b;
}

/* Test class clone logic.  */
static ptr_t
test_clone (const ptr_t self, ptr_t clone)
{
  test_class_t *test_class = (test_class_t *) self;
  test_class_t *test_clone = (test_class_t *) clone;
  test_clone->c = (int_t *) fz_malloc (sizeof (int_t));
  (void) memcpy (test_clone->c, test_class->c, sizeof (int_t));
  return clone;
}

/* Test class compare logic.  */
static int_t
test_compare (const ptr_t self, const ptr_t other)
{
  test_class_t *test_self = (test_class_t *) self;
  test_class_t *test_other = (test_class_t *) other;
  int_t self_value = test_self->a + test_self->b*10;
  int_t other_value = test_other->a + test_other->b*10;
  return self_value - other_value;
}

/* Test class descriptor.  */
static const class_t _TEST_ = {
  sizeof (test_class_t),
  test_constructor,
  test_destructor,
  test_length,
  test_clone,
  test_compare
};

static const class_t _TEST2_ = {
  sizeof (second_class_t),
  NULL, NULL, NULL, NULL, NULL
};

/* Pre-test hook.  */
void
setup ()
{
  ck_assert_int_eq (fz_memusage (0), 0);
  srand (time (0));
  int_t a = rand ();
  int_t b = rand ();
  int_t c = rand ();
  test_instance = fz_new (&_TEST_, a, b, c);
  ck_assert (test_instance->a == a);
  ck_assert (test_instance->b == b);
  ck_assert (*test_instance->c == c);
}

/* Post-test hook.  */
void
teardown ()
{
  ck_assert (fz_del (test_instance) == 0);
  ck_assert_int_eq (fz_memusage (0), 0);
}

/* Test for `fz_new'.  */
START_TEST (test_fz_new)
{
  /* `fz_new' is tested in more detail in `setup' and `teardown'.  */
  ck_assert (fz_new (NULL) == NULL);
}
END_TEST

/* Test for `fz_del'.  */
START_TEST (test_fz_del)
{
  int_t usage = fz_memusage (0);
  test_class_t *instance = fz_new (&_TEST_, 0, 0);
  ck_assert (fz_memusage (0) > usage);
  (void) fz_retain (instance);
  test_destuctor_called = 0;
  ck_assert (fz_del (instance) == 1);
  ck_assert (test_destuctor_called == 0);
  ck_assert (fz_del (instance) == 0);
  ck_assert (test_destuctor_called == 1);
  ck_assert (fz_memusage (0) == usage);
  ck_assert (fz_del (NULL) == -EINVAL);
}
END_TEST

/* Test for `fz_len'.  */
START_TEST (test_fz_len)
{
  size_t length = test_instance->a + test_instance->b;
  ck_assert (fz_len (test_instance) == length);
  ck_assert (fz_len (NULL) == 0);
}
END_TEST

/* Test for `fz_clone'.  */
START_TEST (test_fz_clone)
{
  size_t memusage = fz_memusage (0);
  test_class_t *test_clone = fz_clone (test_instance);
  ck_assert (test_clone != test_instance);
  ck_assert (test_clone->a == test_instance->a);
  ck_assert (test_clone->b == test_instance->b);
  ck_assert (test_clone->c != test_instance->c);
  ck_assert (*test_clone->c == *test_instance->c);
  ck_assert (fz_memusage (0) == memusage * 2);
  ck_assert (fz_del (test_clone) == 0);
  ck_assert (fz_memusage (0) == memusage);
  ck_assert (fz_clone (NULL) == NULL);
}
END_TEST

/* Test for `fz_cmp'.  */
START_TEST (test_fz_cmp)
{
  test_class_t *a = NULL, *b = NULL;
  ck_assert (fz_cmp (a, b) == 0);
  a = test_instance;
  ck_assert (fz_cmp (a, b) > 0);
  b = a, a = NULL;
  ck_assert (fz_cmp (a, b) < 0);
  a = b;
  ck_assert (fz_cmp (a, b) == 0);
  b = fz_clone (a);
  ck_assert (fz_cmp (a, b) == 0);
  a->a += 1;
  ck_assert (fz_cmp (a, b) > 0);
  b->b += 1;
  ck_assert (fz_cmp (a, b) < 0);
  fz_del (b);
}
END_TEST

/* Test for `fz_instance_of'.  */
START_TEST (test_fz_instance_of)
{
  test_class_t *obj1 = fz_new (&_TEST_, 0, 0);
  second_class_t *obj2 = fz_new (&_TEST2_);

  ck_assert (fz_instance_of (NULL, NULL) == FALSE);
  ck_assert (fz_instance_of (obj1, NULL) == FALSE);
  ck_assert (fz_instance_of (NULL, &_TEST_) == FALSE);
  ck_assert (fz_instance_of (obj1, &_TEST2_) == FALSE);
  ck_assert (fz_instance_of (obj2, &_TEST_) == FALSE);
  ck_assert (fz_instance_of (obj1, &_TEST_) == TRUE);
  ck_assert (fz_instance_of (obj2, &_TEST2_) == TRUE);

  fz_del (obj1);
  fz_del (obj2);
}
END_TEST

/* Initiate a class test suite struct.  */
Suite *
class_suite_create ()
{
  Suite *s = suite_create ("class");
  TCase *t = tcase_create ("class");
  tcase_add_checked_fixture (t, setup, teardown);
  tcase_add_test (t, test_fz_new);
  tcase_add_test (t, test_fz_del);
  tcase_add_test (t, test_fz_len);
  tcase_add_test (t, test_fz_clone);
  tcase_add_test (t, test_fz_cmp);
  tcase_add_test (t, test_fz_instance_of);
  suite_add_tcase (s, t);
  return s;
}

/* Run all class tests.  */
int
main ()
{
  int fail_count = 0;
  Suite *suite = class_suite_create ();
  SRunner *runner = srunner_create (suite);
  srunner_run_all (runner, CK_NORMAL);
  fail_count = srunner_ntests_failed (runner);
  srunner_free (runner);
  free (suite);
  return fail_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
