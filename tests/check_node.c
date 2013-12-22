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
#include "private-node.h"
#include "list.h"

/* Test subclass of `node_c'.  */
typedef struct test_node_s
{
  node_t __parent;
  real_t multiplier;
} test_node_t;

static int_t
test_node_render (node_t *node, const request_t *request)
{
  (void) request;
  uint_t i;
  size_t num_frames = fz_len (node->framebuf);
  real_t multiplier = ((test_node_t *) node)->multiplier;

  for (i = 0; i < num_frames; ++i)
    fz_val_at (node->framebuf, i, real_t) *= multiplier;
}

static ptr_t
test_node_constructor (ptr_t ptr, va_list *args)
{
  test_node_t *self = (test_node_t *)
    ((const class_t *) node_c)->construct (ptr, args);
  self->multiplier = va_arg (*args, real_t);
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
}
END_TEST

/* Test for `fz_node_join'.  */
START_TEST (test_fz_node_join)
{
  node_t *node1 = fz_new (node_c);
  node_t *node2 = fz_new (node_c);
  node_t *node3 = fz_new (node_c);

  ck_assert (fz_node_join (NULL, NULL) == -EINVAL);
  ck_assert (fz_node_join (node1, NULL) == -EINVAL);
  ck_assert (fz_node_join (NULL, root_node) == -EINVAL);
  ck_assert (fz_node_join (node1, root_node) == -EINVAL);

  fz_node_fork (root_node, node1);

  ck_assert (fz_node_join (node1, root_node) == -EINVAL);
  ck_assert (fz_node_join (node1, node2) == -EINVAL);

  fz_node_fork (root_node, node2);
  ck_assert (fz_node_join (node1, node2) >= 0);
  ck_assert (fz_node_join (node1, node2) == -EINVAL);
  ck_assert (fz_node_join (node2, node1) == -EINVAL);

  fz_node_fork (node1, node3);
  ck_assert (fz_node_join (node3, root_node) >= 0);
}
END_TEST

/* Test `fz_node_render'.  */
START_TEST (test_fz_node_render)
{
  srand (time (0));

  list_t *frames = fz_new_simple_vector (real_t);
  size_t nframes = 5;
  real_t src[] = {rand (), rand (), rand (), rand (), rand ()};
  request_t request;
  uint_t i;

  node_t *node1 = fz_new (test_node_c, 1.1);
  node_t *node2 = fz_new (test_node_c, 2.2);
  node_t *node3 = fz_new (test_node_c, 3.3);

  fz_node_fork (root_node, node1);
  fz_node_fork (root_node, node2);
  fz_node_fork (node2, node3);

  fz_insert (frames, 0, nframes, src);
  ck_assert (fz_node_render (root_node,
                             frames,
                             &request) == nframes);

  for (i = 0; i < nframes; ++i)
    {
      ck_assert (fz_val_at (frames, i, real_t)
                 == (src[i] * 1.1) + (src[i] * 2.2 * 3.3));
    }

  fz_del (frames);
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
  tcase_add_test (t, test_fz_node_render);
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
