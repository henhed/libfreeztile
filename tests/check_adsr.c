/* Tests for `adsr.h' interface.
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
#include <errno.h>
#include <stdio.h>
#include "malloc.h"
#include "adsr.h"
#include "mod.h"
#include "voice.h"
#include "list.h"
#include "node.h"
#include "form.h"

/* `adsr_c' instance instantiated in `setup'.  */
adsr_t *adsr = NULL;

/* Pre-test hook.  */
void
setup ()
{
  ck_assert_int_eq (fz_memusage (0), 0);
  adsr = fz_new (adsr_c);
}

/* Post-test hook.  */
void
teardown ()
{
  ck_assert (fz_del (adsr) == 0);
  ck_assert_int_eq (fz_memusage (0), 0);
}

/* Test ADSR getters / setters.  */
START_TEST (test_adsr_get_set)
{
  /* Test valid values.  */
  srand (time (0));
  real_t in_range = (real_t) (rand () % 100) / 100;
  ck_assert (fz_adsr_set_a_len (adsr, in_range) == 0);
  ck_assert (fz_adsr_get_a_len (adsr) == in_range);
  ck_assert (fz_adsr_set_a_amp (adsr, in_range) == 0);
  ck_assert (fz_adsr_get_a_amp (adsr) == in_range);
  ck_assert (fz_adsr_set_d_len (adsr, in_range) == 0);
  ck_assert (fz_adsr_get_d_len (adsr) == in_range);
  ck_assert (fz_adsr_set_d_amp (adsr, in_range) == 0);
  ck_assert (fz_adsr_get_d_amp (adsr) == in_range);
  ck_assert (fz_adsr_set_s_len (adsr, in_range) == 0);
  ck_assert (fz_adsr_get_s_len (adsr) == in_range);
  ck_assert (fz_adsr_set_s_amp (adsr, in_range) == 0);
  ck_assert (fz_adsr_get_s_amp (adsr) == in_range);
  ck_assert (fz_adsr_set_r_len (adsr, in_range) == 0);
  ck_assert (fz_adsr_get_r_len (adsr) == in_range);

  /* Test invalid values.  */
  ck_assert (fz_adsr_set_a_len (adsr, -0.1) == EINVAL);
  ck_assert (fz_adsr_get_a_len (adsr) == in_range);
  ck_assert (fz_adsr_set_a_amp (adsr, -0.1) == EINVAL);
  ck_assert (fz_adsr_set_a_amp (adsr, 1.1) == EINVAL);
  ck_assert (fz_adsr_get_a_amp (adsr) == in_range);
  ck_assert (fz_adsr_set_d_len (adsr, -0.1) == EINVAL);
  ck_assert (fz_adsr_get_d_len (adsr) == in_range);
  ck_assert (fz_adsr_set_d_amp (adsr, -0.1) == EINVAL);
  ck_assert (fz_adsr_set_d_amp (adsr, 1.1) == EINVAL);
  ck_assert (fz_adsr_get_d_amp (adsr) == in_range);
  ck_assert (fz_adsr_set_s_len (adsr, -0.1) == EINVAL);
  ck_assert (fz_adsr_get_s_len (adsr) == in_range);
  ck_assert (fz_adsr_set_s_amp (adsr, -0.1) == EINVAL);
  ck_assert (fz_adsr_set_s_amp (adsr, 1.1) == EINVAL);
  ck_assert (fz_adsr_get_s_amp (adsr) == in_range);
  ck_assert (fz_adsr_set_r_len (adsr, -0.1) == EINVAL);
  ck_assert (fz_adsr_get_r_len (adsr) == in_range);
}
END_TEST

/* Test the ADSR modulator realization.  */
START_TEST (test_adsr_render)
{
  FILE *tsv = fopen ("check_adsr.dat", "w");
  const char *header = "\"Form\"\t\"Envelope\"\n";
  char val[16];
  size_t nframes = 150;
  node_t *form = fz_new (form_c, SHAPE_SINE);
  list_t *frames = fz_new_simple_vector (real_t);
  request_t request = REQUEST_DEFAULT (fz_new (voice_c));
  const list_t *env;
  uint_t i;
  int_t err;

  fz_set_sample_rate (nframes * 2);

  fz_adsr_set_a_len (adsr, 0.10);
  fz_adsr_set_a_amp (adsr, 1.00);
  fz_adsr_set_d_len (adsr, 0.10);
  fz_adsr_set_d_amp (adsr, 0.50);
  fz_adsr_set_s_len (adsr, 0.20);
  fz_adsr_set_s_amp (adsr, 0.75);
  fz_adsr_set_r_len (adsr, 0.40);

  fz_node_connect ((node_t *) form, (mod_t *) adsr,
                   FORM_SLOT_AMP, NULL);

  ck_assert (fz_adsr_is_silent (adsr, request.voice) == TRUE);
  fz_voice_press (request.voice, 20, 0.8);

  fz_clear (frames, nframes);
  fz_node_render ((node_t *) form, frames, &request);
  ck_assert (fz_adsr_is_silent (adsr, request.voice) == FALSE);
  env = fz_modulate_unorm ((mod_t *) adsr, 1);

  fputs (header, tsv);
  for (i = 0; i < nframes; ++i)
    {
      sprintf (val, "%.4f\t", fz_val_at (frames, i, real_t));
      fputs (val, tsv);
      sprintf (val, "%.4f\n", fz_val_at (env, i, real_t));
      fputs (val, tsv);
    }

  fz_voice_release (request.voice);
  fz_clear (frames, nframes);
  fz_node_render ((node_t *) form, frames, &request);
  ck_assert (fz_adsr_is_silent (adsr, request.voice) == TRUE);
  env = fz_modulate_unorm ((mod_t *) adsr, 1);

  for (i = 0; i < nframes; ++i)
    {
      sprintf (val, "%.4f\t", fz_val_at (frames, i, real_t));
      fputs (val, tsv);
      sprintf (val, "%.4f\n", fz_val_at (env, i, real_t));
      fputs (val, tsv);
    }

  fz_mod_prepare ((mod_t *) adsr, nframes);
  err = fz_mod_render ((mod_t *) adsr, &request);
  fail_unless (err == nframes,
               "Expected ADSR to render %u frames but it returned %d.",
               (unsigned) nframes, err);

  fz_del (request.voice);
  request.voice = NULL;

  fz_mod_prepare ((mod_t *) adsr, nframes);
  err = fz_mod_render ((mod_t *) adsr, &request);
  fail_unless (err < 0,
               "Expected ADSR to fail with NULL voice but got '%d'.",
               err);

  fz_del (frames);
  fz_del (form);
  fclose (tsv);
}
END_TEST

/* Initiate an ADSR test suite struct.  */
Suite *
adsr_suite_create ()
{
  Suite *s = suite_create ("adsr");
  TCase *t = tcase_create ("adsr");
  tcase_add_checked_fixture (t, setup, teardown);
  tcase_add_test (t, test_adsr_get_set);
  tcase_add_test (t, test_adsr_render);
  suite_add_tcase (s, t);
  return s;
}

/* Run all ADSR tests.  */
int
main ()
{
  int fail_count = 0;
  Suite *suite = adsr_suite_create ();
  SRunner *runner = srunner_create (suite);
  srunner_run_all (runner, CK_NORMAL);
  fail_count = srunner_ntests_failed (runner);
  srunner_free (runner);
  free (suite);
  return fail_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
