/* Tests for `class.c' functions.
   Copyright (C) 2013 Henrik Hedelund.

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
#include <check.h>
#include <errno.h>
#include "class.h"

/* Definition of a test class with two integer members.  */
typedef struct {
  class_t *__class;
  int_t a;
  int_t b;
} test_class_t;

/* Test class instance instantiated in `setup'.  */
test_class_t *test_instance = NULL;

/* Flag used to assert destructor invocation.  */
int_t test_destuctor_called = 0;

/* Test class constructor.  */
static ptr_t
test_constructor (ptr_t ptr, va_list *args)
{
  test_class_t *self = (test_class_t *) ptr;
  self->a = va_arg (args, int_t);
  self->b = va_arg (args, int_t);
  return self;
}

/* Test class destructor.   */
static ptr_t
test_destructor (ptr_t ptr)
{
  test_destuctor_called = 1;
  return ptr;
}

/* Test class descriptor.  */
static const class_t _TEST_ = {
  sizeof (test_class_t),
  test_constructor,
  test_destructor,
  NULL,
  NULL,
  NULL
};

/* Pre-test hook.  */
void
setup ()
{
  ck_assert_int_eq (fz_memusage (0), 0);
  srand (time (0));
  int_t a = rand ();
  int_t b = rand ();
  test_instance = fz_new (&_TEST_, a, b);
  ck_assert (test_instance->a == a);
  ck_assert (test_instance->b == b);
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

/* Initiate a class test suite struct.  */
Suite *
class_suite_create ()
{
  Suite *s = suite_create ("class");
  TCase *t = tcase_create ("class");
  tcase_add_checked_fixture (t, setup, teardown);
  tcase_add_test (t, test_fz_new);
  tcase_add_test (t, test_fz_del);
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
