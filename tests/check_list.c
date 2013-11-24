/* Tests for `list.c' functions.
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
#include "list.h"

/* `vector_c' instance instantiated in `setup'.  */
list_t *test_vector = NULL;

/* Pre-test hook.  */
void
setup ()
{
  ck_assert_int_eq (fz_memusage (0), 0);
  test_vector = fz_new (vector_c, sizeof (int), "int");
}

/* Post-test hook.  */
void
teardown ()
{
  ck_assert (fz_del (test_vector) == 0);
  ck_assert_int_eq (fz_memusage (0), 0);
}

/* Test for `fz_at'.  */
START_TEST (test_fz_at)
{
  ck_assert (fz_at (NULL, 0) == NULL);
  ck_assert (fz_at (test_vector, 0) == NULL);
  /* More assertions in tests below.  */
}
END_TEST

/* Test for `fz_insert'.  */
START_TEST (test_fz_insert)
{
  srand (time (0));
  size_t i;
  size_t num_vals = 128 + (rand () % 128);
  int vals[num_vals];
  ck_assert (fz_len (test_vector) == 0);

  /* Test inserting single values.  */
  for (i = 0; i < num_vals; ++i)
    {
      vals[i] = rand ();
      fz_insert (test_vector, 0, 1, vals + i);
      ck_assert (*((int *) fz_at (test_vector, 0)) == vals[i]);
      ck_assert (fz_len (test_vector) == i + 1);
    }

  ck_assert (fz_insert (test_vector, 0, 1, NULL) == -EINVAL);

  /* Test inserting multiple values at some offset.  */
  fz_insert (test_vector, 5, 10, vals);
  for (i = 0; i < 10; ++i)
    {
      ck_assert_int_eq (*((int *) fz_at (test_vector, i + 5)), vals[i]);
    }
  ck_assert_int_eq (fz_len (test_vector), num_vals + 10);

  /* Test inserting at end.  */
  fz_insert (test_vector, fz_len (test_vector), 1, vals);
  ck_assert_int_eq (*((int *) fz_at (test_vector, fz_len (test_vector) - 1)),
		    vals[0]);
  ck_assert_int_eq (fz_len (test_vector), num_vals + 11);

  /* Test inserting past end.  */
  ck_assert (fz_insert (test_vector,
			fz_len (test_vector) + 1,
			1,
			vals) == -EINVAL);
  ck_assert_int_eq (fz_len (test_vector), num_vals + 11);
}
END_TEST

/* Test for `fz_erase'.  */
START_TEST (test_fz_erase)
{
  srand (time (0));
  int vals[] = {rand (), rand (), rand ()};
  ck_assert (fz_erase (test_vector, 0) == -EINVAL);
  fz_insert (test_vector, 0, 3, vals);
  ck_assert (fz_erase (test_vector, 2) == 2);
  ck_assert (fz_erase (test_vector, 2) == -EINVAL);
  ck_assert (*((int *) fz_at (test_vector, 0)) == vals[0]);
  ck_assert (fz_erase (test_vector, 0) == 0);
  ck_assert (*((int *) fz_at (test_vector, 0)) == vals[1]);
  ck_assert (fz_erase (test_vector, 0) == 0);
  ck_assert (fz_at (test_vector, 0) == NULL);
  ck_assert (fz_erase (test_vector, 0) == -EINVAL);
}
END_TEST

/* Initiate a list test suite struct.  */
Suite *
list_suite_create ()
{
  Suite *s = suite_create ("list");
  TCase *t = tcase_create ("list");
  tcase_add_checked_fixture (t, setup, teardown);
  tcase_add_test (t, test_fz_at);
  tcase_add_test (t, test_fz_insert);
  tcase_add_test (t, test_fz_erase);
  suite_add_tcase (s, t);
  return s;
}

/* Run all list tests.  */
int
main ()
{
  int fail_count = 0;
  Suite *suite = list_suite_create ();
  SRunner *runner = srunner_create (suite);
  srunner_run_all (runner, CK_NORMAL);
  fail_count = srunner_ntests_failed (runner);
  srunner_free (runner);
  free (suite);
  return fail_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
