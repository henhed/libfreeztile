/* Tests for `form.c' functions.
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

#include <check.h>
#include "form.h"
#include "node.h"

/* Pre-test hook.  */
void
setup ()
{
  ck_assert_int_eq (fz_memusage (0), 0);
}

/* Post-test hook.  */
void
teardown ()
{
  ck_assert_int_eq (fz_memusage (0), 0);
}

/* Test of triangle shaped form.  */
START_TEST (test_triangle_form)
{
  form_t *triangle = fz_new (form_c);
  list_t *frames = fz_new_simple_vector (real_t);
  size_t num_frames = 256;
  uint_t i;

  fz_clear (frames, num_frames);
  ck_assert (fz_node_render (triangle, frames) == num_frames);

  fz_del (frames);
  fz_del (triangle);
}
END_TEST

/* Initiate a form test suite struct.  */
Suite *
form_suite_create ()
{
  Suite *s = suite_create ("form");
  TCase *t = tcase_create ("form");
  tcase_add_checked_fixture (t, setup, teardown);
  tcase_add_test (t, test_triangle_form);
  suite_add_tcase (s, t);
  return s;
}

/* Run all form tests.  */
int
main ()
{
  int fail_count = 0;
  Suite *suite = form_suite_create ();
  SRunner *runner = srunner_create (suite);
  srunner_run_all (runner, CK_NORMAL);
  fail_count = srunner_ntests_failed (runner);
  srunner_free (runner);
  free (suite);
  return fail_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
