/* Tests for `filter.c' functions.
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
#include <stdio.h>
#include "filter.h"
#include "node.h"
#include "form.h"

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

static inline real_t
get_peak_amplitude (const filter_t *filter, real_t frequency)
{
  form_t *form = fz_new (form_c, SHAPE_SINE);
  request_t request = REQUEST_DEFAULT (fz_new (voice_c));
  list_t *frames = fz_new_simple_vector (real_t);
  size_t nframes = ((real_t) request.srate / frequency) * 50;

  /* Render FILTERed sine into FRAMES.  */
  fz_voice_press (request.voice, frequency, 1);
  fz_clear (frames, nframes);
  fz_node_render ((node_t *) form, frames, &request);
  ck_assert (fz_node_render ((node_t *) filter,
                             frames,
                             &request) == nframes);

  /* Find peak amplitude.  */
  real_t peak = 0;
  real_t val;
  uint_t i;
  for (i = nframes / 2; i < nframes; ++i)
    {
      val = fz_val_at (frames, i, real_t);
      if (val < 0)
        val *= -1;
      if (val > peak)
        peak = val;
    }

  fz_del (request.voice);
  fz_del (form);
  fz_del (frames);

  return peak;
}

/* Test filter output.  */
START_TEST (test_filter)
{
  FILE *tsv = fopen ("check_filter.dat", "w");
  size_t nbands = 20;
  uint_t i, j, k;
  const char *types[] = {"Lowpass", "Highpass", "Bandpass"};
  for (i = 5; i <= nbands; i += 5)
    {
      for (j = 0; j < 3; ++j)
        fprintf (tsv, "\"%s: Cutoff @ %d kHz\"%s", types[j], i,
                 j == 2 && i == nbands ? "\n" : "\t");
    }

  filter_t *filter = fz_new (filter_c);
  fz_filter_set_resonance (filter, 0.3);

  /* Plot amplitude response up to half sample rate.  */
  for (i = 100; i < REQUEST_SRATE_DEFAULT / 2; i += 100)
    {
      /* Plot cutoff frequencies up to 20 kHz.  */
      for (j = 5; j <= nbands; j += 5)
        {
          fz_filter_set_frequency (filter, j * 1000.);
          /* Plot low-, high- and bandpass.  */
          for (k = 0; k < 3; ++k)
            {
              fz_filter_set_type (filter, k);
              fprintf (tsv, k == 2 && j == nbands ? "%.4f\n" : "%.4f\t",
                       get_peak_amplitude (filter, (real_t) i));
            }
        }
    }

  fz_del (filter);
}
END_TEST

/* Initiate a filter test suite struct.  */
Suite *
filter_suite_create ()
{
  Suite *s = suite_create ("filter");
  TCase *t = tcase_create ("filter");
  tcase_add_checked_fixture (t, setup, teardown);
  tcase_add_test (t, test_filter);
  suite_add_tcase (s, t);
  return s;
}

/* Run all filter tests.  */
int
main ()
{
  int fail_count = 0;
  Suite *suite = filter_suite_create ();
  SRunner *runner = srunner_create (suite);
  srunner_run_all (runner, CK_NORMAL);
  fail_count = srunner_ntests_failed (runner);
  srunner_free (runner);
  free (suite);
  return fail_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
