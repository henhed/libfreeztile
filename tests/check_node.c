/* Tests for `node.c' functions.
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
#include <time.h>
#include <errno.h>
#include "malloc.h"
#include "node.h"
#include "private-node.h"
#include "mod.h"
#include "private-mod.h"
#include "list.h"

#define TEST_MOD_SLOT 1

/* Test subclass of `node_c'.  */
typedef struct test_node_s
{
  node_t __parent;
  real_t multiplier;
} test_node_t;

static int_t
test_node_render (node_t *node, list_t *frames,
                  const voice_t *voice)
{
  (void) voice;
  uint_t i;
  size_t nframes = fz_len (frames);
  real_t multiplier = ((test_node_t *) node)->multiplier;
  real_t *modarg;
  const real_t *moddata;

  modarg = fz_node_modargs (node, TEST_MOD_SLOT);
  moddata = fz_node_modulate_unorm (node, TEST_MOD_SLOT,
                                    modarg ? *modarg : 1);

  for (i = 0; i < nframes; ++i)
    {
      fz_val_at (frames, i, real_t) *= multiplier;
      if (moddata != NULL)
        fz_val_at (frames, i, real_t) *= moddata[i];
    }

  return nframes;
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

/* Test subclass of `mod_c'.  */
typedef struct test_mod_s
{
  mod_t __parent;
} test_mod_t;

static int_t
test_mod_render (mod_t *mod, const voice_t *voice)
{
  uint_t i;
  size_t size = fz_len (mod->stepbuf);
  for (i = 0; i < size; ++i)
    fz_val_at (mod->stepbuf, i, real_t) = ((real_t) i) / size;
  return size;
}

static ptr_t
test_mod_constructor (ptr_t ptr, va_list *args)
{
  (void) args;
  test_mod_t *self = (test_mod_t *)
    ((const class_t *) mod_c)->construct (ptr, args);
  self->__parent.render = test_mod_render;
  return self;
}

static ptr_t
test_mod_destructor (ptr_t ptr)
{
  test_mod_t *self = (test_mod_t *)
    ((const class_t *) mod_c)->destruct (ptr);
  return self;
}

static const class_t _test_mod_c = {
  sizeof (test_mod_t),
  test_mod_constructor,
  test_mod_destructor,
  NULL,
  NULL,
  NULL
};

const class_t *test_mod_c = &_test_mod_c;
/* End test subclass of `mod_c'.  */

/* `node_c' instance instantiated in `setup'.  */
node_t *test_node = NULL;

/* Pre-test hook.  */
void
setup ()
{
  ck_assert_int_eq (fz_memusage (0), 0);
  srand (time (0));
  test_node = fz_new (test_node_c, ((real_t) (rand () % 100)) / 100);
}

/* Post-test hook.  */
void
teardown ()
{
  ck_assert (fz_del (test_node) == 0);
  ck_assert_int_eq (fz_memusage (0), 0);
}

/* Test `fz_node_render'.  */
START_TEST (test_fz_node_render)
{
  list_t *frames = fz_new_simple_vector (real_t);
  size_t nframes = 5;
  real_t src[] = {rand (), rand (), rand (), rand (), rand ()};
  real_t multi = ((test_node_t *) test_node)->multiplier;
  uint_t i;

  fz_insert (frames, 0, nframes, src);
  ck_assert (fz_node_render (test_node,
                             frames,
                             NULL) == nframes);

  for (i = 0; i < nframes; ++i)
    {
      ck_assert (fz_val_at (frames, i, real_t) == src[i] * multi);
    }

  fz_del (frames);
}
END_TEST

/* Test node-modulator connection.  */
START_TEST (test_fz_node_connect)
{
  list_t *frames = fz_new_simple_vector (real_t);
  size_t nframes = 5;
  real_t src[] = {rand (), rand (), rand (), rand (), rand ()};
  real_t multi = ((test_node_t *) test_node)->multiplier;
  mod_t *mod = fz_new (test_mod_c);
  list_t *mods = fz_new_owning_vector (mod_t *);
  real_t modarg = 2.2;
  int_t lhs, rhs;
  uint_t i;
  int_t err;

  fz_insert (frames, 0, nframes, src);

  err = fz_node_connect (NULL, mod, 0, NULL);
  fail_unless (err > 0,
               "Expected connect to fail with NULL node but got '%d'",
               err);

  err = fz_node_connect (test_node, NULL, 0, NULL);
  fail_unless (err > 0,
               "Expected connect to fail with NULL mod but got '%d'",
               err);

  fz_node_collect_mods (test_node, mods);
  fail_unless (fz_len (mods) == 0,
               "Expected node to have 0 mods but it had '%d'",
               fz_len (mods));

  err = fz_node_connect (test_node, mod, TEST_MOD_SLOT, &modarg);
  fail_unless (err == 0,
               "Expected connect to succeed but got '%d'",
               err);

  fz_node_collect_mods (test_node, mods);
  fail_unless (fz_len (mods) == 1,
               "Expected node to have 1 mod but it had '%d'",
               fz_len (mods));

  fz_mod_prepare (mod, nframes);
  fz_mod_render (mod, NULL);
  fz_node_prepare (test_node, nframes);
  fz_node_render (test_node, frames, NULL);
  for (i = 0; i < nframes; ++i)
    {
      /* The test mod should generate a saw from 0 to `modarg'.  */
      lhs = (int_t) fz_val_at (frames, i, real_t);
      rhs = (int_t) ((((real_t) i) / nframes) * modarg * src[i] * multi);
      fail_unless (lhs == rhs,
                   "Expected modulated output to be %d but got %d",
                   lhs, rhs);
    }

  /* Add same mod to a second slot.  */
  fz_node_connect (test_node, mod, TEST_MOD_SLOT + 1, &modarg);
  fz_node_collect_mods (test_node, mods);
  fail_unless (fz_len (mods) == 1,
               "Expected node to have 1 unique mod but got '%d'",
               fz_len (mods));

  /* Add new mod to a third slot.  */
  fz_node_connect (test_node, fz_new (test_mod_c), TEST_MOD_SLOT + 2,
                   NULL);
  fz_node_collect_mods (test_node, mods);
  fail_unless (fz_len (mods) == 2,
               "Expected node to have 2 unique mods but got '%d'",
               fz_len (mods));

  fz_del (mods);
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
  tcase_add_test (t, test_fz_node_render);
  tcase_add_test (t, test_fz_node_connect);
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
