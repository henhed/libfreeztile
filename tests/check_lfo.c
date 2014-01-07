/* Tests for `lfo.h' interface.
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
#include <stdio.h>
#include "lfo.h"
#include "form.h"
#include "mod.h"
#include "voice.h"
#include "node.h"
#include "list.h"

/* `lfo_c' instance instantiated in `setup'.  */
lfo_t *lfo = NULL;

/* Pre-test hook.  */
void
setup ()
{
  ck_assert_int_eq (fz_memusage (0), 0);
  lfo = fz_new (lfo_c, SHAPE_SINE, (real_t) 1);
}

/* Post-test hook.  */
void
teardown ()
{
  ck_assert (fz_del (lfo) == 0);
  ck_assert_int_eq (fz_memusage (0), 0);
}

/* Test the LFO modulator realization.  */
START_TEST (test_lfo_render)
{
  FILE *tsv = fopen ("check_lfo.dat", "w");
  const char *header = "\"Form\"\t\"LFO\"\t\"FM Form\"\n";
  char val[16];
  request_t request = REQUEST_DEFAULT (fz_new (voice_c));
  size_t nframes = 300;
  node_t *form = fz_new (form_c, SHAPE_SINE);
  node_t *fmform = fz_new (form_c, SHAPE_SINE);
  list_t *frames = fz_new_simple_vector (real_t);
  list_t *fmframes = fz_new_simple_vector (real_t);
  const list_t *lfoframes;
  real_t depth = 3;
  int_t err;
  uint_t i;

  fz_clear (frames, nframes);
  fz_clear (fmframes, nframes);
  fz_node_connect ((node_t *) fmform, (mod_t *) lfo,
                   FORM_SLOT_FREQ, &depth);

  fz_voice_press (request.voice, 2, 1);
  request.srate = nframes / 2;
  fz_node_render ((node_t *) form, frames, &request);
  fz_node_render ((node_t *) fmform, fmframes, &request);
  /* Modulation is already rendered in `fz_node_render'.  */
  lfoframes = fz_modulate_snorm ((mod_t *) lfo, 1);

  fputs (header, tsv);
  for (i = 0; i < nframes; ++i)
    {
      sprintf (val, "%.4f\t", fz_val_at (frames, i, real_t));
      fputs (val, tsv);
      sprintf (val, "%.4f\t", fz_val_at (lfoframes, i, real_t));
      fputs (val, tsv);
      sprintf (val, "%.4f\n", fz_val_at (fmframes, i, real_t));
      fputs (val, tsv);
    }

  fz_mod_prepare ((mod_t *) lfo, nframes);
  err = fz_mod_render ((mod_t *) lfo, &request);
  fail_unless (err == nframes,
               "Expected LFO to render %u frames but it returned %d.",
               (unsigned) nframes, err);

  fz_del (request.voice);
  request.voice = NULL;

  fz_mod_prepare ((mod_t *) lfo, nframes);
  err = fz_mod_render ((mod_t *) lfo, &request);
  fail_unless (err < 0,
               "Expected LFO to fail with NULL voice but got '%d'.",
               err);

  fz_del (fmframes);
  fz_del (frames);
  fz_del (fmform);
  fz_del (form);
  fclose (tsv);
}
END_TEST

/* Initiate an LFO test suite struct.  */
Suite *
lfo_suite_create ()
{
  Suite *s = suite_create ("lfo");
  TCase *t = tcase_create ("lfo");
  tcase_add_checked_fixture (t, setup, teardown);
  tcase_add_test (t, test_lfo_render);
  suite_add_tcase (s, t);
  return s;
}

/* Run all LFO tests.  */
int
main ()
{
  int fail_count = 0;
  Suite *suite = lfo_suite_create ();
  SRunner *runner = srunner_create (suite);
  srunner_run_all (runner, CK_NORMAL);
  fail_count = srunner_ntests_failed (runner);
  srunner_free (runner);
  free (suite);
  return fail_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
