/* Tests for `graph.c' functions.
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

#include <check.h>
#include "graph.h"

/* `graph_c' instance instantiated in `setup'.  */
graph_t *test_graph = NULL;

/* Pre-test hook.  */
void
setup ()
{
  ck_assert_int_eq (fz_memusage (0), 0);
  test_graph = fz_new (graph_c);
}

/* Post-test hook.  */
void
teardown ()
{
  ck_assert (fz_del (test_graph) == 0);
  ck_assert_int_eq (fz_memusage (0), 0);
}

/* Test for `fz_graph_add_node' / `fz_graph_del_node'.  */
START_TEST (test_fz_graph_add_node)
{
  node_t *node = fz_new (node_c);

  ck_assert (fz_graph_add_node (NULL, NULL) != 0);
  ck_assert (fz_graph_add_node (test_graph, NULL) != 0);
  ck_assert (fz_graph_add_node (NULL, node) != 0);

  ck_assert (fz_graph_has_node (test_graph, node) == FALSE);
  ck_assert (fz_graph_add_node (test_graph, node) == 0);
  ck_assert (fz_graph_has_node (test_graph, node) == TRUE);
  ck_assert (fz_graph_add_node (test_graph, node) != 0);

  ck_assert (fz_graph_del_node (NULL, NULL) != 0);
  ck_assert (fz_graph_del_node (test_graph, NULL) != 0);
  ck_assert (fz_graph_del_node (NULL, node) != 0);

  ck_assert (fz_graph_del_node (test_graph, node) == 0);
  ck_assert (fz_graph_has_node (test_graph, node) == FALSE);
  ck_assert (fz_graph_del_node (test_graph, node) != 0);

  fz_del (node);
}
END_TEST

/* Initiate a graph test suite struct.  */
Suite *
graph_suite_create ()
{
  Suite *s = suite_create ("graph");
  TCase *t = tcase_create ("graph");
  tcase_add_checked_fixture (t, setup, teardown);
  tcase_add_test (t, test_fz_graph_add_node);
  suite_add_tcase (s, t);
  return s;
}

/* Run all graph tests.  */
int
main ()
{
  int fail_count = 0;
  Suite *suite = graph_suite_create ();
  SRunner *runner = srunner_create (suite);
  srunner_run_all (runner, CK_NORMAL);
  fail_count = srunner_ntests_failed (runner);
  srunner_free (runner);
  free (suite);
  return fail_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
