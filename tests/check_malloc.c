/* Tests for `malloc.c' functions.
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
#include "malloc.h"

/* Test variables initiated in `setup'.  */
static ptr_t test_ptr = NULL;
static size_t test_ptr_size = 0;

/* The meta struct declaration used by the fz_* memory functions is not
   publically available so we maintain our own definition for testing.  */
struct memory_meta_clone
{
  uint_t numref;
  size_t len;
};

/* Pre-test hook.  */
void
setup ()
{
  ck_assert (fz_memusage (0) == 0);
  srand (time (0));
  test_ptr_size = rand () % 256;
  test_ptr = fz_malloc (test_ptr_size);
  ck_assert (test_ptr != NULL);
}

/* Post-test hook.  */
void
teardown ()
{
  ck_assert (fz_free (test_ptr) == 0);
  ck_assert (fz_memusage (0) == 0);
  test_ptr = NULL;
  test_ptr_size = 0;
}

/* Test for `fz_realloc'.  */
START_TEST (test_fz_realloc)
{
  ptr_t ptr;
  ptr = fz_realloc (test_ptr, test_ptr_size * 2);
  ck_assert (ptr != NULL);
  if (ptr != NULL && ptr != test_ptr)
    test_ptr = ptr;
}
END_TEST

/* Test for `fz_retain'.  */
START_TEST (test_fz_retain)
{
  ck_assert (fz_retain (test_ptr) == test_ptr);
  ck_assert (fz_retain (test_ptr) == test_ptr);
  ck_assert (fz_free (test_ptr) == 2);
  ck_assert (fz_free (test_ptr) == 1);
  ck_assert (fz_retain (NULL) == NULL);
}
END_TEST

/* Test for `fz_refcount'.  */
START_TEST (tets_fz_refcount)
{
  int_t count = fz_refcount (test_ptr);
  (void) fz_retain (test_ptr);
  ck_assert (fz_refcount (test_ptr) == count + 1);
  (void) fz_free (test_ptr);
  ck_assert (fz_refcount (test_ptr) == count);
  ck_assert (fz_refcount (NULL) == -EINVAL);
}
END_TEST

/* Test for `fz_free'.  */
START_TEST (test_fz_free)
{
  /* `fz_free' is tested in `teardown' and `fz_retain' test as well.  */
  ck_assert (fz_free (NULL) == -EINVAL);
}
END_TEST

/* Test for `fz_memusage'.  */
START_TEST (test_fz_memusage)
{
  size_t meta_size = sizeof (struct memory_meta_clone);
  size_t usage;

  /* Check memory stats for activities in `setup'.  */
  ck_assert (fz_memusage (MEMUSAGE_META) == meta_size);
  ck_assert (fz_memusage (MEMUSAGE_DISP) == test_ptr_size);

  /* Reallocing should not change the meta size.  */
  test_ptr = fz_realloc (test_ptr, test_ptr_size * 2);
  ck_assert (fz_memusage (MEMUSAGE_META) == meta_size);
  ck_assert (fz_memusage (MEMUSAGE_DISP) >= test_ptr_size * 2);

  /* Reallocing a smaller space should not affect anything.  */
  usage = fz_memusage (MEMUSAGE_DISP);
  test_ptr = fz_realloc (test_ptr, test_ptr_size);
  ck_assert (fz_memusage (MEMUSAGE_META) == meta_size);
  ck_assert (fz_memusage (MEMUSAGE_DISP) == usage);

  /* Make sure different flavors of 'show all' reports the same usage.  */
  usage = fz_memusage (0);
  ck_assert ((fz_memusage (MEMUSAGE_META | MEMUSAGE_DISP) == usage)
	     && (fz_memusage (MEMUSAGE_All) == usage));
}
END_TEST

/* Initiate a malloc test suite struct.  */
Suite *
malloc_suite_create ()
{
  Suite *s = suite_create ("malloc");
  TCase *t = tcase_create ("malloc");
  tcase_add_checked_fixture (t, setup, teardown);
  tcase_add_test (t, test_fz_realloc);
  tcase_add_test (t, test_fz_retain);
  tcase_add_test (t, tets_fz_refcount);
  tcase_add_test (t, test_fz_free);
  tcase_add_test (t, test_fz_memusage);
  suite_add_tcase (s, t);
  return s;
}

/* Run all malloc tests.  */
int
main ()
{
  int fail_count = 0;
  Suite *suite = malloc_suite_create ();
  SRunner *runner = srunner_create (suite);
  srunner_run_all (runner, CK_NORMAL);
  fail_count = srunner_ntests_failed (runner);
  srunner_free (runner);
  free (suite);
  return fail_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
