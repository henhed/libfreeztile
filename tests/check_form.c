/* Tests for `form.c' functions.
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
#include <stdio.h>
#include "form.h"
#include "node.h"

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

/* Test form shapes.  */
START_TEST (test_form_shapes)
{
  /* The correctness of shapes are hard to measure prorgamatically
     so we render them into a CSV file so they can be inspected in
     external chart tools.  */
  FILE *csv = fopen ("check_form.csv", "w");
  size_t nforms = 3;
  form_t *forms[] = {
    fz_new (form_c, SHAPE_SINE),
    fz_new (form_c, SHAPE_TRIANGLE),
    fz_new (form_c, SHAPE_SQUARE)
  };
  list_t *framebufs[] = {
    fz_new_simple_vector (real_t),
    fz_new_simple_vector (real_t),
    fz_new_simple_vector (real_t)
  };
  size_t nframes = 256;
  request_t request = {
    .voice = NULL,
    .srate = REQUEST_SRATE_DEFAULT,
    .access = REQUEST_ACCESS_INTERLEAVED
  };
  char val[16]; /* Needs to hold at least "d.dd,\0" (a CSV cell).  */
  uint_t i, j;

  /* Render forms to frame buffers.  */
  for (i = 0; i < nforms; ++i)
    {
      request.voice = fz_new (voice_c);
      fz_voice_press (request.voice, 440, 1);

      fz_clear (framebufs[i], nframes);
      ck_assert (fz_node_render ((node_t *) forms[i],
                                 framebufs[i],
                                 &request) == nframes);
      fz_del (request.voice);
      fz_del (forms[i]);
    }

  /* Write frame buffers to file.  */
  for (i = 0; i < nframes; ++i)
    {
      for (j = 0; j < nforms; ++j)
        {
          sprintf (val,
                   j == nforms - 1 ? "%.2f\n" : "%.2f,",
                   fz_val_at (framebufs[j], i, real_t));
          fwrite (val, sizeof (char), strlen (val), csv);
        }
    }

  for (i = 0; i < nforms; ++i)
    fz_del (framebufs[i]);

  fclose (csv);
}
END_TEST

/* Initiate a form test suite struct.  */
Suite *
form_suite_create ()
{
  Suite *s = suite_create ("form");
  TCase *t = tcase_create ("form");
  tcase_add_checked_fixture (t, setup, teardown);
  tcase_add_test (t, test_form_shapes);
  suite_add_tcase (s, t);
  return s;
}

/* Run all form tests.  */
int
main ()
{
  int fail_count = 0;
  Suite *suite = form_suite_create ();
  SRunner *runner = srunner_create (suite);
  srunner_run_all (runner, CK_NORMAL);
  fail_count = srunner_ntests_failed (runner);
  srunner_free (runner);
  free (suite);
  return fail_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
