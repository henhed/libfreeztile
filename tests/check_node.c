/* Tests for `node.c' functions.
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
#include <errno.h>
#include "node.h"

/* `node_c' instance instantiated in `setup'.  */
node_t *root_node = NULL;

/* Pre-test hook.  */
void
setup ()
{
  ck_assert_int_eq (fz_memusage (0), 0);
  root_node = fz_new (node_c);
}

/* Post-test hook.  */
void
teardown ()
{
  ck_assert (fz_del (root_node) == 0);
  ck_assert_int_eq (fz_memusage (0), 0);
}

/* Test for `fz_node_fork'.  */
START_TEST (test_fz_node_fork)
{
  node_t *node1 = fz_new (node_c);
  node_t *node2 = fz_new (node_c);
  node_t *node3 = fz_new (node_c);

  ck_assert (fz_node_fork (root_node, node1) == 0);
  ck_assert (fz_node_fork (root_node, node2) == 1);
  ck_assert (fz_node_fork (node2, node3) == 0);

  ck_assert (fz_node_fork (root_node, node1) == -EINVAL);
  ck_assert (fz_node_fork (node2, node1) == -EINVAL);
  ck_assert (fz_node_fork (node3, node1) == -EINVAL);
  ck_assert (fz_node_fork (node3, root_node) == -EINVAL);
  ck_assert (fz_node_fork (node2, node2) == -EINVAL);
  ck_assert (fz_node_fork (NULL, node2) == -EINVAL);
  ck_assert (fz_node_fork (node2, NULL) == -EINVAL);

  ck_assert (fz_del (node1) == 0);
  ck_assert (fz_del (node2) == 0);
  ck_assert (fz_del (node3) == 0);
}
END_TEST

/* Test for `fz_node_join'.  */
START_TEST (test_fz_node_join)
{

}
END_TEST

/* Initiate a node test suite struct.  */
Suite *
node_suite_create ()
{
  Suite *s = suite_create ("node");
  TCase *t = tcase_create ("node");
  tcase_add_checked_fixture (t, setup, teardown);
  tcase_add_test (t, test_fz_node_fork);
  tcase_add_test (t, test_fz_node_join);
  suite_add_tcase (s, t);
  return s;
}

/* Run all node tests.  */
int
main ()
{
  int fail_count = 0;
  Suite *suite = node_suite_create ();
  SRunner *runner = srunner_create (suite);
  srunner_run_all (runner, CK_NORMAL);
  fail_count = srunner_ntests_failed (runner);
  srunner_free (runner);
  free (suite);
  return fail_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
