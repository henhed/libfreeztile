/* Tests for `map.h' functions.
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

#include <stdlib.h>
#include <time.h>
#include <check.h>
#include "map.h"
#include "malloc.h"
#include "list.h"

/* Test map item struct.  */
typedef struct {
  uint_t value;
  size_t size;
  ptr_t data;
} item_t;

/* Test map instance.  */
static map_t *test_map;

/* Test map item initializer.  */
void
init_item (const map_t *map, uintptr_t key, ptr_t item)
{
  item_t *_item = (item_t *) item;
  _item->value = (uint_t) (key * key);
  _item->size = (rand () % 100) + 1;
  _item->data = fz_malloc (_item->size);
}

/* Test map item clean-up callback.  */
void
free_item (const map_t *map, uintptr_t key, ptr_t item)
{
  item_t *_item = (item_t *) item;
  fz_free (_item->data);
  ck_assert (_item->value == (uint_t) (key * key));
}

/* Pre-test hook.  */
void
setup ()
{
  srand (time (0));
  ck_assert_int_eq (fz_memusage (0), 0);
  test_map = fz_new (map_c, NULL, init_item, free_item);
}

/* Post-test hook.  */
void
teardown ()
{
  fz_del (test_map);
  ck_assert_int_eq (fz_memusage (0), 0);
}

/* Test map owner function.  */
START_TEST (test_map_owner)
{
  ck_assert (fz_map_owner (test_map) == NULL);
  map_t *map = fz_new (map_c, test_map, NULL, NULL);
  ck_assert (fz_map_owner (map) == test_map);
  fz_del (map);
}
END_TEST

/* Test map get / set / unset functions.  */
START_TEST (test_map_getsetuns)
{
  uint_t i;
  size_t nitems = 100;
  for (i = 1; i <= nitems; ++i)
    {
      uintptr_t key = i * 2;
      uint_t value = key * key;

      /* Test get before set.  */
      ck_assert (fz_map_get (test_map, key) == NULL);

      /* Test set.  */
      item_t *item = fz_map_set (test_map, key, NULL,
                                 sizeof (item_t));
      fail_unless (fz_len (test_map) == i,
                   "Expected map length to be %u but got %lu",
                   i, fz_len (test_map));
      fail_unless (item->value == value,
                   "Expected value %u but got %u",
                   item->value, value);

      /* Test get after set.  */
      ck_assert (fz_map_get (test_map, key) == item);
      ck_assert (fz_map_get (test_map, key - 1) == NULL);

      /* Test unset.  */
      fz_map_unset (test_map, key);
      ck_assert (fz_map_get (test_map, key) == NULL);
      fail_unless (fz_len (test_map) == i - 1,
                   "Expected map length to be %u but got %lu",
                   i - 1, fz_len (test_map));

      fz_map_set (test_map, key, NULL, sizeof (item_t));
    }

  ck_assert (fz_map_get (NULL, 0) == NULL);
  ck_assert (fz_map_set (NULL, 0, NULL, 0) == NULL);
}
END_TEST

/* Test variations of map set function.  */
START_TEST (test_map_setval)
{
  map_t *map = fz_new (map_c, NULL, NULL, NULL);
  uintptr_t key = (uintptr_t) test_map;

  /* Test set with zero size, should copy pointer address.  */
  fz_map_set (map, key, test_map, 0);
  map_t **map_ptr = fz_map_get (map, key);
  ck_assert (*map_ptr == test_map);

  /* Test set with value size, should copy value.  */
  int value = rand ();
  fz_map_set (map, key, &value, sizeof (int));
  int *val_ptr = fz_map_get (map, key);
  ck_assert (value == *val_ptr);

  /* Test set with NULL value, should memset to 0.  */
  fz_map_set (map, key, NULL, sizeof (int));
  val_ptr = fz_map_get (map, key);
  ck_assert (*val_ptr == 0);

  fz_del (map);
}
END_TEST

/* Test `fz_map_key`.  */
START_TEST (test_map_key)
{
  uint_t i;
  size_t nitems = 100;
  for (i = 0; i < nitems; ++i)
    {
      uintptr_t key = (uintptr_t) rand ();
      item_t *item = fz_map_set (test_map, key, NULL,
                                 sizeof (item_t));
      fail_unless (fz_map_key (item) == key,
                   "Expected key to be %lu but got %lu",
                   fz_map_key (item), key);
    }
}
END_TEST

/* Test `fz_map_next`.  */
START_TEST (test_map_next)
{
  uint_t i;
  size_t nitems = 100;
  item_t *item;
  uintptr_t keysum = 0;
  uint_t valsum;

  /* Populate map and calculate key / value sums.  */
  for (i = 0; i < nitems; ++i)
    {
      item = fz_map_set (test_map, i, NULL, sizeof (item_t));
      keysum += i;
      valsum += item->value;
    }

  /* Iterate through map and subtract sums.  */
  fz_map_each (test_map, item)
    {
      keysum -= fz_map_key (item);
      valsum -= item->value;
    }

  /* Assert sums are zero.  */
  ck_assert (keysum == 0);
  ck_assert (valsum == 0);
}
END_TEST

/* Initiate a map test suite struct.  */
Suite *
map_suite_create ()
{
  Suite *s = suite_create ("map");
  TCase *t = tcase_create ("map");
  tcase_add_checked_fixture (t, setup, teardown);
  tcase_add_test (t, test_map_owner);
  tcase_add_test (t, test_map_getsetuns);
  tcase_add_test (t, test_map_setval);
  tcase_add_test (t, test_map_key);
  tcase_add_test (t, test_map_next);
  suite_add_tcase (s, t);
  return s;
}

/* Run all map tests.  */
int
main ()
{
  int fail_count = 0;
  Suite *suite = map_suite_create ();
  SRunner *runner = srunner_create (suite);
  srunner_run_all (runner, CK_NORMAL);
  fail_count = srunner_ntests_failed (runner);
  srunner_free (runner);
  free (suite);
  return fail_count == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
