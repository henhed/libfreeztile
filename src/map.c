/* Implementation of map functions.
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

#include <string.h>
#include <errno.h>
#include "map.h"
#include "malloc.h"

#ifndef MAP_INDEX_SIZE
#define MAP_INDEX_SIZE 53
#endif

/* Internal map item struct.  */
typedef struct item_s item_t;
struct item_s {
  uintptr_t key;
  item_t *next;
  size_t size;
};

/* Map class struct.  */
struct map_s
{
  const class_t *__class;
  item_t *index[MAP_INDEX_SIZE];
  ptr_t owner;
  map_value_f init_value;
  map_value_f free_value;
};

/* Map constuctor.  */
static ptr_t
map_constructor (ptr_t ptr, va_list *args)
{
  map_t *map = (map_t *) ptr;
  memset (map->index, 0, sizeof (item_t *) * MAP_INDEX_SIZE);
  map->owner = va_arg (*args, ptr_t);
  map->init_value = va_arg (*args, map_value_f);
  map->free_value = va_arg (*args, map_value_f);
  return map;
}

/* Map destructor.  */
static ptr_t
map_destructor (ptr_t ptr)
{
  map_t *map = (map_t *) ptr;

  uint_t i;
  item_t *item = NULL, *prev = NULL;
  for (i = 0; i < MAP_INDEX_SIZE; ++i)
    {
      item = map->index[i];
      while (item)
        {
          if (map->free_value)
            map->free_value (map, item->key, item + 1);
          prev = item;
          item = prev->next;
          fz_free (prev);
        }
      map->index[i] = NULL;
    }

  return map;
}

/* `fz_len' implementation.  */
static size_t
map_length (const ptr_t map)
{
  size_t length = 0;
  ptr_t value = fz_map_next (map, NULL);
  for (; value != NULL; value = fz_map_next (map, value))
    ++length;
  return length;
}

/* Get given MAPs owner.  */
ptr_t
fz_map_owner (const map_t *map)
{
  return map ? map->owner : NULL;
}

/* Get item mapped to given KEY or create it if it doesn't exist and
   SIZE is greater than zero.  */
static item_t *
map_get_item (map_t *map, uintptr_t key, size_t size, bool_t *is_new)
{
  if (!map)
    {
      errno = EINVAL;
      return NULL;
    }

  uint_t i = key % MAP_INDEX_SIZE;
  item_t *item = map->index[i], *prev = NULL;
  while (item && item->key != key)
    {
      prev = item;
      item = item->next;
    }

  if (item)
    {
      if (size > item->size)
        {
          item = fz_realloc (item, sizeof (item_t) + size);
          item->size = size;
          if (prev)
            prev->next = item;
          else
            map->index[i] = item;
        }

      if (is_new)
        *is_new = FALSE;
    }
  else if (!item && size > 0)
    {
      item = fz_malloc (sizeof (item_t) + size);
      item->key = key;
      item->next = NULL;
      item->size = size;
      if (prev)
        prev->next = item;
      else
        map->index[i] = item;

      if (is_new)
        *is_new = TRUE;
    }

  return item;
}

/* Get data mapped to given KEY in MAP.  */
ptr_t
fz_map_get (const map_t *map, uintptr_t key)
{
  item_t *item = map_get_item ((map_t *) map, key, 0, NULL);
  return item ? item + 1 : NULL;
}

/* Map given VALUE of size SIZE to KEY.  */
ptr_t
fz_map_set (map_t *map, uintptr_t key, const ptr_t value, size_t size)
{
  if (!map)
    {
      errno = EINVAL;
      return NULL;
    }

  ptr_t _value = (ptr_t) value;
  if (size == 0)
    {
      size = sizeof (ptr_t);
      _value = (ptr_t) &value;
    }

  bool_t is_new = FALSE;
  item_t *item = map_get_item (map, key, size, &is_new);
  if (item)
    {
      if (!is_new && map->free_value)
        map->free_value (map, item->key, item + 1);

      if (value)
        memcpy (item + 1, _value, size);
      else
        memset (item + 1, 0, size);

      if (map->init_value)
        map->init_value (map, item->key, item + 1);

      return item + 1;
    }

  return NULL;
}

/* Remove value with given KEY in MAP.  */
void
fz_map_unset (map_t *map, uintptr_t key)
{
  if (!map)
    return;

  uint_t i = key % MAP_INDEX_SIZE;
  item_t *item = map->index[i], *prev = NULL;
  while (item && item->key != key)
    {
      prev = item;
      item = item->next;
    }

  if (item)
    {
      if (prev)
        prev->next = item->next;
      else
        map->index[i] = item->next;

      if (map->free_value)
        map->free_value (map, item->key, item + 1);

      fz_free (item);
    }
}

/* Get the key of given VALUE in MAP.  */
uintptr_t
fz_map_key (const ptr_t value)
{
  if (value)
    return (((item_t *) value) - 1)->key;
  errno = EINVAL;
  return 0;
}

/* Get next value in relation to PREV from MAP.  */
ptr_t
fz_map_next (const map_t *map, const ptr_t prev)
{
  if (!map)
    {
      errno = EINVAL;
      return NULL;
    }

  uint_t i = 0;
  if (prev)
    {
      item_t *item = ((item_t *) prev) - 1;
      if (item->next)
        return item->next + 1;
      else
        i = (item->key % MAP_INDEX_SIZE) + 1;
    }

  for (; i < MAP_INDEX_SIZE; ++i)
    if (map->index[i])
      return map->index[i] + 1;

  return NULL;
}

/* `map_c' class descriptor.  */
static const class_t _map_c = {
  sizeof (map_t),
  map_constructor,
  map_destructor,
  map_length,
  NULL,
  NULL
};

const class_t *map_c = &_map_c;
