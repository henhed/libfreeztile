/* Tests for `delay.h' functions.
   Copyright (C) 2013-2015 Henrik Hedelund.

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
#include <stdio.h>
#include <errno.h>
#include "delay.h"
#include "node.h"
#include "list.h"

typedef real_t (*getter_f) (const delay_t *);
typedef int_t (*setter_f) (delay_t *, real_t);

/* Pre-test hook.  */
void
setup ()
{
  srand (time (0));
  ck_assert_int_eq (fz_memusage (0), 0);
}

/* Post-test hook.  */
void
teardown ()
{
  ck_assert_int_eq (fz_memusage (0), 0);
}

/* Test delay class setters and getters.  */
START_TEST (test_delay_setget)
{
  delay_t *delay = fz_new (delay_c);
  real_t rnd = (real_t) (rand () % 101) / 100.;
  int_t err;
  real_t value;
  uint_t i;
  size_t nprops = 3;
  const char *props[] = {
    "gain",
    "feedback",
    "delay"
  };
  getter_f getters[] = {
    fz_delay_get_gain,
    fz_delay_get_feedback,
    fz_delay_get_delay
  };
  setter_f setters[] = {
    fz_delay_set_gain,
    fz_delay_set_feedback,
    fz_delay_set_delay
  };
  
  for (i = 0; i < nprops; ++i)
    {
      err = setters[i] (delay, rnd);
      fail_unless (err == 0,
                   "Expected %s %.2f to be accepted but got error %d",
                   props[i], rnd, err);
      value = getters[i] (delay);
      fail_unless (value == rnd,
                   "Expected %s to be %.2f but got %.2f",
                   props[i], rnd, value);
      err = setters[i] (delay, rnd - 1.01);
      fail_unless (err == EINVAL,
                   "Expected negative %s %.2f to return %d but got %d",
                   props[i], rnd - 1., EINVAL, err);
      value = getters[i] (delay);
      fail_unless (value == rnd,
                   "Expected %s to remain %.2f but got %.2f",
                   props[i], rnd, value);
    }

  err = fz_delay_set_feedback (delay, rnd + 1.01);
  fail_unless (err == EINVAL,
               "Expected feedback %.2f to return %d but got %d",
               props[i], rnd + 1.1, EINVAL, err);

  fz_del (delay);
}
END_TEST

/* Test delay output.  */
START_TEST (test_delay_output)
{
  FILE *tsv = fopen ("check_delay.dat", "w");
  fprintf (tsv, "\"Wet\"\t\"Dry\"\n");
  delay_t *delay = fz_new (delay_c);
  voice_t *voice = fz_new (voice_c);
  list_t *frames = fz_new_simple_vector (real_t);
  list_t *frames_copy = fz_new_simple_vector (real_t);
  size_t nframes = 1000;
  uint_t i;
  real_t dry, wet;

  /* Generate input signal.  */
  fz_clear (frames, nframes);
  for (i = 1; i < nframes / 2; i += 100)
    {
      fz_val_at (frames, i - 1, real_t) = 1.0;
      fz_val_at (frames, i, real_t) = -1.0;
    }
  fz_insert (frames_copy, 0, nframes, fz_list_data (frames));

  /* Apply delay to signal.  */
  fz_delay_set_gain (delay, 0.5);
  fz_delay_set_feedback (delay, 0.75);
  fz_delay_set_delay (delay,
                      (nframes / fz_get_sample_rate ()) * 0.031);
  ck_assert (fz_node_render ((node_t *) delay, frames, voice)
             == nframes);

  /* Write to file.  */
  for (i = 0; i < nframes; ++i)
    {
      dry = fz_val_at (frames_copy, i, real_t);
      wet = fz_val_at (frames, i, real_t) - dry;
      fprintf (tsv, "%.4f\t%.4f\n", wet, dry);
    }

  fz_del (frames_copy);
  fz_del (frames);
  fz_del (voice);
  fz_del (delay);
  fclose (tsv);
}
END_TEST

/* Initiate a delay test suite struct.  */
Suite *
delay_suite_create ()
{
  Suite *s = suite_create ("delay");
  TCase *t = tcase_create ("delay");
  tcase_add_checked_fixture (t, setup, teardown);
  tcase_add_test (t, test_delay_setget);
  tcase_add_test (t, test_delay_output);
  suite_add_tcase (s, t);
  return s;
}

/* Run all delay tests.  */
int
main ()
{
  int fail_count = 0;
  Suite *suite = delay_suite_create ();
  SRunner *runner = srunner_create (suite);
  srunner_run_all (runner, CK_NORMAL);
  fail_count = srunner_ntests_failed (runner);
  srunner_free (runner);
  free (suite);
  return fail_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
