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
#include "malloc.h"
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


/* Test for `fz_graph_connect'.  */
START_TEST (test_fz_graph_connect)
{
  /*
    Sample directed acyclic graph where a, f, and g are sources and b,
    c and e are sinks.

           +---+  +---+
           | a |  | b |<------+
           +---+  +---+       |
             |      ^         |
             | +----+         |
             V |              |
    +---+   +---+   +---+   +---+
    | c |<--| d |-->| e |   | f |
    +---+   +---+   +---+   +---+
              ^       ^       |
              |       |       |
            +---+   +---+     |
            | g |-->| h |<----+
            +---+   +---+
   */

  node_t *a = fz_new (node_c);
  node_t *b = fz_new (node_c);
  node_t *c = fz_new (node_c);
  node_t *d = fz_new (node_c);
  node_t *e = fz_new (node_c);
  node_t *f = fz_new (node_c);
  node_t *g = fz_new (node_c);
  node_t *h = fz_new (node_c);

  /* Test EINVAL.  */
  ck_assert (fz_graph_connect (NULL, NULL, NULL) != 0);
  ck_assert (fz_graph_connect (test_graph, NULL, NULL) != 0);
  ck_assert (fz_graph_connect (NULL, a, NULL) != 0);
  ck_assert (fz_graph_connect (NULL, NULL, b) != 0);
  ck_assert (fz_graph_connect (test_graph, a, NULL) != 0);
  ck_assert (fz_graph_connect (test_graph, NULL, b) != 0);
  ck_assert (fz_graph_connect (NULL, a, b) != 0);
  ck_assert (fz_graph_connect (test_graph, a, b) != 0);

  fz_graph_add_node (test_graph, a);
  fz_graph_add_node (test_graph, b);
  fz_graph_add_node (test_graph, c);
  fz_graph_add_node (test_graph, d);
  fz_graph_add_node (test_graph, e);
  fz_graph_add_node (test_graph, f);
  fz_graph_add_node (test_graph, g);
  fz_graph_add_node (test_graph, h);

  /* Make sure we're allowed to create the example edges.  */
  ck_assert (fz_graph_connect (test_graph, a, d) == 0);
  ck_assert (fz_graph_connect (test_graph, d, b) == 0);
  ck_assert (fz_graph_connect (test_graph, d, c) == 0);
  ck_assert (fz_graph_connect (test_graph, d, e) == 0);
  ck_assert (fz_graph_connect (test_graph, f, b) == 0);
  ck_assert (fz_graph_connect (test_graph, f, h) == 0);
  ck_assert (fz_graph_connect (test_graph, g, d) == 0);
  ck_assert (fz_graph_connect (test_graph, g, h) == 0);
  ck_assert (fz_graph_connect (test_graph, h, e) == 0);

  /* Make sure we're *not* allowed to make any loops.  */
  /* a-d-c-a */
  ck_assert (fz_graph_can_connect (test_graph, c, a) == FALSE);
  /* g-d-c-g */
  ck_assert (fz_graph_can_connect (test_graph, c, g) == FALSE);
  /* f-h-e-f */
  ck_assert (fz_graph_can_connect (test_graph, e, f) == FALSE);
  /* d-c-d */
  ck_assert (fz_graph_can_connect (test_graph, c, d) == FALSE);
  /* a-a */
  ck_assert (fz_graph_can_connect (test_graph, a, a) == FALSE);

  fz_del (h);
  fz_del (g);
  fz_del (f);
  fz_del (e);
  fz_del (d);
  fz_del (c);
  fz_del (b);
  fz_del (a);
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
  tcase_add_test (t, test_fz_graph_connect);
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
