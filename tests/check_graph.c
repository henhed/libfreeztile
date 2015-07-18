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
#include "private-node.h"

/* Test subclass of `node_c'.  */
typedef struct test_node_s
{
  node_t __parent;
  real_t sample;
} test_node_t;

static int_t
test_node_render (node_t *node, list_t *frames,
                  const voice_t *voice)
{
  (void) voice;
  real_t sample = ((test_node_t *) node)->sample;
  uint_t i;
  size_t nframes = fz_len (frames);
  for (i = 0; i < nframes; ++i)
    fz_val_at (frames, i, real_t) += sample;
  return nframes;
}

static ptr_t
test_node_constructor (ptr_t ptr, va_list *args)
{
  test_node_t *self = (test_node_t *)
    ((const class_t *) node_c)->construct (ptr, args);
  self->sample = va_arg (*args, real_t);
  self->__parent.render = test_node_render;
  return self;
}

static ptr_t
test_node_destructor (ptr_t ptr)
{
  test_node_t *self = (test_node_t *)
    ((const class_t *) node_c)->destruct (ptr);
  return self;
}

static const class_t _test_node_c = {
  sizeof (test_node_t),
  test_node_constructor,
  test_node_destructor,
  NULL,
  NULL,
  NULL
};

const class_t *test_node_c = &_test_node_c;
/* End test subclass of `node_c'.  */

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

/* Test for `fz_graph_render'.  */
START_TEST (test_fz_graph_render)
{
  real_t sample1 = 1;
  real_t sample2 = 2;
  real_t sample3 = 3;
  int_t nframes = 10;
  node_t *in1 = fz_new (test_node_c, sample1);
  node_t *in2 = fz_new (test_node_c, sample2);
  node_t *in3 = fz_new (test_node_c, sample3);
  node_t *out1 = fz_new (node_c);
  node_t *out2 = fz_new (node_c);

  fz_graph_add_node (test_graph, in1);
  fz_graph_add_node (test_graph, in2);
  fz_graph_add_node (test_graph, in3);
  fz_graph_add_node (test_graph, out1);
  fz_graph_add_node (test_graph, out2);

  /* in1 -> in2 -> out1 (sample1 + sample2) */
  fz_graph_connect (test_graph, in1, in2);
  fz_graph_connect (test_graph, in2, out1);
  /* in1 -> in3 -> out2 (sample1 + sample3) */
  fz_graph_connect (test_graph, in1, in3);
  fz_graph_connect (test_graph, in3, out2);

  ck_assert (fz_graph_prepare (test_graph, nframes) == 0);
  ck_assert_int_eq (fz_graph_render (test_graph, NULL), nframes);

  const list_t *outbuf1 = fz_graph_buffer (test_graph, out1);
  const list_t *outbuf2 = fz_graph_buffer (test_graph, out2);
  ck_assert (fz_len ((const ptr_t) outbuf1) == (size_t) nframes);
  ck_assert (fz_len ((const ptr_t) outbuf2) == (size_t) nframes);

  int_t frame = 0;
  for (; frame < nframes; ++frame)
    {
      real_t out1sample = fz_val_at (outbuf1, frame, real_t);
      real_t out2sample = fz_val_at (outbuf2, frame, real_t);
      ck_assert (out1sample == sample1 + sample2);
      ck_assert (out2sample == sample1 + sample3);
    }

  fz_del (out2);
  fz_del (out1);
  fz_del (in3);
  fz_del (in2);
  fz_del (in1);
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
  tcase_add_test (t, test_fz_graph_render);
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
