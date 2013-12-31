/* Tests for `mod.h' interface.
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
#include "mod.h"
#include "private-mod.h"
#include "class.h"

/* Sample subclass of `mod_c'.  */
typedef struct sample_mod_s
{
  mod_t __parent;
} sample_mod_t;

static const class_t _sample_mod_c = {
  sizeof (sample_mod_t),
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

const class_t *sample_mod_c = &_sample_mod_c;
/* End of `mod_c' sample subclass.  */

/* `mod_c' instance instantiated in `setup'.  */
mod_t *modulator = NULL;

/* Pre-test hook.  */
void
setup ()
{
  ck_assert_int_eq (fz_memusage (0), 0);
  modulator = fz_new (sample_mod_c);
}

/* Post-test hook.  */
void
teardown ()
{
  ck_assert (fz_del (modulator) == 0);
  ck_assert_int_eq (fz_memusage (0), 0);
}

/* Initiate a modulator test suite struct.  */
Suite *
mod_suite_create ()
{
  Suite *s = suite_create ("modulator");
  TCase *t = tcase_create ("modulator");
  tcase_add_checked_fixture (t, setup, teardown);
  suite_add_tcase (s, t);
  return s;
}

/* Run all modulator tests.  */
int
main ()
{
  int fail_count = 0;
  Suite *suite = mod_suite_create ();
  SRunner *runner = srunner_create (suite);
  srunner_run_all (runner, CK_NORMAL);
  fail_count = srunner_ntests_failed (runner);
  srunner_free (runner);
  free (suite);
  return fail_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
