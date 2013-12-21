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

/* Initiate a voice test suite struct.  */
Suite *
voice_suite_create ()
{
  Suite *s = suite_create ("voice");
  TCase *t = tcase_create ("voice");
  tcase_add_checked_fixture (t, setup, teardown);
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
