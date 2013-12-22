/* Tests for `voice.c' functions.
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
#include <stdlib.h>
#include <errno.h>
#include "voice.h"

voice_t *voice = NULL;

/* Pre-test hook.  */
void
setup ()
{
  ck_assert_int_eq (fz_memusage (0), 0);
  voice = fz_new (voice_c);
}

/* Post-test hook.  */
void
teardown ()
{
  fz_del (voice);
  ck_assert_int_eq (fz_memusage (0), 0);
}

/* Positive Voice press, aftertouch and release function tests.  */
START_TEST (test_fz_voice_press_pos)
{
  srand (time (0));

  int_t (*pfnc) (voice_t *, real_t, real_t) = fz_voice_press;
  int_t (*afnc) (voice_t *, real_t) = fz_voice_aftertouch;
  int_t (*rfnc) (voice_t *) = fz_voice_release;
  static const char *pname = "fz_voice_press";
  static const char *aname = "fz_voice_aftertouch";
  static const char *rname = "fz_voice_release";
  int_t err;
  real_t freq = fmax (0.01, rand () % 10000);
  real_t velo = fmax (0.01, ((real_t) rand ()) / RAND_MAX);
  real_t pres = fmax (0.01, ((real_t) rand ()) / RAND_MAX);
  real_t cfreq, cvelo, cpres;
  bool_t pressed;

  /* Press.  */
  err = pfnc (voice, freq, velo);
  pressed = fz_voice_pressed (voice);
  fail_unless (!err,
               "%s didn't accept f = %f and v = %f (errno %d)",
               pname, freq, velo, err);
  fail_unless (pressed, "Expected voice to be pressed after press");
  cfreq = fz_voice_frequency (voice);
  cvelo = fz_voice_velocity (voice);
  cpres = fz_voice_pressure (voice);
  /* Pressure == velocity after press.  */
  fail_unless (cfreq == freq && cvelo == velo && cpres == velo,
               "Expected f = %f, v = %f, p = %f"
               " but got f = %f, v = %f, p = %f",
               freq, velo, velo, cfreq, cvelo, cpres);

  /* Aftertouch.  */
  err = afnc (voice, pres);
  pressed = fz_voice_pressed (voice);
  fail_unless (!err,
               "%s didn't accept p = %f (errno %d)",
               aname, pres, err);
  fail_unless (pressed, "Expected voice to be pressed after touch");
  cfreq = fz_voice_frequency (voice);
  cvelo = fz_voice_velocity (voice);
  cpres = fz_voice_pressure (voice);
  fail_unless (cfreq == freq && cvelo == velo && cpres == pres,
               "Expected f = %f, v = %f, p = %f"
               " but got f = %f, v = %f, p = %f",
               freq, velo, pres, cfreq, cvelo, cpres);

  /* Release.  */
  err = rfnc (voice);
  pressed = fz_voice_pressed (voice);
  fail_unless (!err, "%s didn't release (errno %d)", rname, err);
  fail_unless (!pressed,
               "Expected voice not to be pressed after release");
}
END_TEST

/* Initiate a voice test suite struct.  */
Suite *
voice_suite_create ()
{
  Suite *s = suite_create ("voice");
  TCase *t = tcase_create ("voice");
  tcase_add_checked_fixture (t, setup, teardown);
  tcase_add_test (t, test_fz_voice_press_pos);
  suite_add_tcase (s, t);
  return s;
}

/* Run all voice tests.  */
int
main ()
{
  int fail_count = 0;
  Suite *suite = voice_suite_create ();
  SRunner *runner = srunner_create (suite);
  srunner_run_all (runner, CK_NORMAL);
  fail_count = srunner_ntests_failed (runner);
  srunner_free (runner);
  free (suite);
  return fail_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
