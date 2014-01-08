/* Tests for `adsr.h' interface.
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
#include "adsr.h"
#include "mod.h"
#include "voice.h"
#include "list.h"

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

/* Test the ADSR modulator realization.  */
START_TEST (test_adsr_render)
{
  FILE *tsv = fopen ("check_adsr.dat", "w");
  const char *header = "\"Envelope\"\n";
  char val[16];
  size_t nframes = 150;
  request_t request = REQUEST_DEFAULT (fz_new (voice_c));
  const list_t *env;
  uint_t i;
  int_t err;

  request.srate = nframes * 2;
  fz_voice_press (request.voice, 440, 0.8);

  fz_mod_prepare ((mod_t *) adsr, nframes);
  fz_mod_render ((mod_t *) adsr, &request);
  env = fz_modulate_unorm ((mod_t *) adsr, 1);

  fputs (header, tsv);
  for (i = 0; i < nframes; ++i)
    {
      sprintf (val, "%.4f\n", fz_val_at (env, i, real_t));
      fputs (val, tsv);
    }

  fz_voice_release (request.voice);
  fz_mod_prepare ((mod_t *) adsr, nframes);
  fz_mod_render ((mod_t *) adsr, &request);
  env = fz_modulate_unorm ((mod_t *) adsr, 1);

  for (i = 0; i < nframes; ++i)
    {
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
