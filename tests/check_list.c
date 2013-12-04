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
  test_vector = fz_new (vector_c, sizeof (int), "int", LISTOPT_NONE);
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
  int vals[] = {rand (), rand (), rand (), rand (), rand ()};
  ck_assert (fz_erase (test_vector, 0, 0) == -EINVAL);
  ck_assert (fz_erase (test_vector, 0, 1) == -EINVAL);
  fz_insert (test_vector, 0, 5, vals);

  /* Test erasing multiple items.  */
  ck_assert (fz_erase (test_vector, 1, 3) == 1);
  ck_assert_int_eq (*((int *) fz_at (test_vector, 0)), vals[0]);
  ck_assert_int_eq (*((int *) fz_at (test_vector, 1)), vals[4]);
  ck_assert (fz_len (test_vector) == 2);

  /* Test erasing past end.  */
  ck_assert (fz_erase (test_vector, fz_len (test_vector), 1) == -EINVAL);
  ck_assert (fz_len (test_vector) == 2);

  /* Test erasing too many items.  */
  ck_assert (fz_erase (test_vector, 1, fz_len (test_vector)) == -EINVAL);
  ck_assert (fz_len (test_vector) == 2);

  /* Test erasing at end.  */
  ck_assert (fz_erase (test_vector, 1, 1) == 1);
  ck_assert (fz_len (test_vector) == 1);
}
END_TEST

/* Test for `list_c' option LISTOPT_PTRS.  */
START_TEST (test_listopt_ptrs)
{
  /* Test normal behavior.  */
  list_t *list = fz_new (vector_c,
			 sizeof (ptr_t),
			 "ptr_t",
			 LISTOPT_NONE);
  ck_assert (list != NULL);

  fz_insert (list, 0, 1, &test_vector);
  ck_assert (*((ptr_t *) fz_at (list, 0)) == test_vector);
  fz_insert (list, 1, 1, test_vector);
  ck_assert (fz_at (list, 1) != test_vector);

  ck_assert (fz_del (list) == 0);

  /* Test pointer storage behavior.  */
  list = fz_new (vector_c,
		 sizeof (ptr_t),
		 "ptr_t",
		 LISTOPT_PTRS);
  ck_assert (list != NULL);

  fz_insert (list, 0, 1, test_vector);
  ck_assert (fz_at (list, 0) == test_vector);

  ck_assert (fz_del (list) == 0);
}
END_TEST

/* Test for `list_c' option LISTOPT_KEEP.  */
START_TEST (test_listopt_keep)
{
  /* Test normal behavior.  */
  list_t *list = fz_new (vector_c,
			 sizeof (ptr_t),
			 "ptr_t",
			 LISTOPT_PTRS);
  list_t *object = fz_new (vector_c,
			 sizeof (int_t),
			 "int_t",
			 LISTOPT_NONE);
  ck_assert (list != NULL && object != NULL);

  fz_insert (list, 0, 1, object);
  size_t memusage = fz_memusage (0);
  fz_del (object);
  ck_assert (fz_memusage (0) < memusage);
  fz_del (list);

  /* Test retaining behavior.  */
  list = fz_new (vector_c,
		 sizeof (ptr_t),
		 "ptr_t",
		 LISTOPT_PTRS | LISTOPT_KEEP);
  object = fz_new (vector_c,
		   sizeof (int_t),
		   "int_t",
		   LISTOPT_NONE);
  ck_assert (list != NULL && object != NULL);

  fz_insert (list, 0, 1, object);
  memusage = fz_memusage (0);
  fz_del (object);
  ck_assert (fz_memusage (0) == memusage);
  fz_erase (list, 0, 1);
  ck_assert (fz_memusage (0) < memusage);

  object = fz_new (vector_c,
		   sizeof (int_t),
		   "int_t",
		   LISTOPT_NONE);
  fz_insert (list, 0, 1, object);
  memusage = fz_memusage (0);
  fz_del (object);
  ck_assert (fz_memusage (0) == memusage);
  fz_del (list);
  /* `teardown' makes a final memusage test.  */
}
END_TEST

/* Test for `fz_index_of'.  */
START_TEST (test_fz_index_of)
{
  /* Test searching for numbers.  */
  int_t first = 1;
  int_t second = 2;
  int_t third = 3;
  int_t fourth = 4;
  fz_insert (test_vector, 0, 1, &first);
  ck_assert (fz_index_of (test_vector, &first, fz_cmp_int) == 0);
  fz_insert (test_vector, 0, 1, &second);
  ck_assert (fz_index_of (test_vector, &second, fz_cmp_int) == 0);
  ck_assert (fz_index_of (test_vector, &first, fz_cmp_int) == 1);
  fz_insert (test_vector, 0, 1, &third);
  ck_assert (fz_index_of (test_vector, &third, fz_cmp_int) == 0);
  ck_assert (fz_index_of (test_vector, &second, fz_cmp_int) == 1);
  ck_assert (fz_index_of (test_vector, &first, fz_cmp_int) == 2);
  fz_insert (test_vector, 0, 1, &first);
  ck_assert (fz_index_of (test_vector, &first, fz_cmp_int) == 0);
  ck_assert (fz_index_of (test_vector, &third, fz_cmp_int) == 1);
  ck_assert (fz_index_of (test_vector, &second, fz_cmp_int) == 2);
  ck_assert (fz_index_of (test_vector, &fourth, fz_cmp_int) < 0);

  /* Test searching for pointers.  */
  list_t *list = fz_new (vector_c,
			 sizeof (ptr_t),
			 "ptr_t",
			 LISTOPT_PTRS);
  list_t *ptr1 = fz_new (vector_c, sizeof (ptr_t), "ptr_t", 0);
  list_t *ptr2 = fz_new (vector_c, sizeof (ptr_t), "ptr_t", 0);
  list_t *ptr3 = fz_new (vector_c, sizeof (ptr_t), "ptr_t", 0);
  fz_insert (list, 0, 1, ptr1);
  ck_assert (fz_index_of (list, ptr1, fz_cmp_ptr) == 0);
  fz_insert (list, 0, 1, ptr2);
  ck_assert (fz_index_of (list, ptr2, fz_cmp_ptr) == 0);
  ck_assert (fz_index_of (list, ptr1, fz_cmp_ptr) == 1);
  fz_insert (list, 0, 1, ptr3);
  ck_assert (fz_index_of (list, ptr3, fz_cmp_ptr) == 0);
  ck_assert (fz_index_of (list, ptr2, fz_cmp_ptr) == 1);
  ck_assert (fz_index_of (list, ptr1, fz_cmp_ptr) == 2);
  fz_insert (list, 0, 1, ptr1);
  ck_assert (fz_index_of (list, ptr1, fz_cmp_ptr) == 0);
  ck_assert (fz_index_of (list, ptr3, fz_cmp_ptr) == 1);
  ck_assert (fz_index_of (list, ptr2, fz_cmp_ptr) == 2);
  fz_del (ptr3);
  fz_del (ptr2);
  fz_del (ptr1);
  fz_del (list);
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
  tcase_add_test (t, test_listopt_ptrs);
  tcase_add_test (t, test_listopt_keep);
  tcase_add_test (t, test_fz_index_of);
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
