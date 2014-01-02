/* Implementation of list functions.
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

#include <errno.h>
#include <string.h>
#include "list.h"
#include "defs.h"
#include "malloc.h"
#include "class.h"

/* Abstract list class struct.  */
struct list_s
{
  const class_t *__class;
  size_t type_size;
  char *type_name;
  flags_t flags;
  int_t (*insert) (list_t *, uint_t, uint_t, ptr_t);
  int_t (*erase) (list_t *, uint_t, uint_t);
  ptr_t (*at) (const list_t *, uint_t);
};

/* Abstract list constuctor.  */
static ptr_t
list_constructor (ptr_t ptr, va_list *args)
{
  list_t *self = (list_t *) ptr;
  size_t type_size = va_arg (*args, size_t);
  const char *type_name = va_arg (*args, char *);
  flags_t flags = va_arg (*args, flags_t);

  self->type_name = (char *) fz_malloc (sizeof (char) *
                                        (strlen (type_name) + 1));
  strcpy (self->type_name, type_name);
  self->type_size = type_size;
  self->flags = flags;
  self->insert = NULL;
  self->erase = NULL;
  self->at = NULL;

  return self;
}

/* Abstract list destructor.  */
static ptr_t
list_destructor (ptr_t ptr)
{
  list_t *self = (list_t *) ptr;
  uint_t i;
  size_t len;

  if ((self->flags & LISTOPT_KEEP) == LISTOPT_KEEP
      || (self->flags & LISTOPT_PASS) == LISTOPT_PASS)
    {
      len = fz_len (self);
      for (i = 0; i < len; ++i)
        fz_del (fz_at (self, i));
    }

  fz_free (self->type_name);
  return self;
}

/* Abstract list item getter.  */
ptr_t
fz_at (const list_t *list, uint_t index)
{
  if (list == NULL
      || list->at == NULL
      || index >= fz_len ((const ptr_t) list))
    return NULL;

  if (list->flags & LISTOPT_PTRS)
    return *((ptr_t *) list->at (list, index));
  else
    return list->at (list, index);
}

/* Insert concrete implementation wrapper.  */
int_t
fz_insert (list_t *list, uint_t index, uint_t num, ptr_t item)
{
  int_t err = -EINVAL;
  bool_t can_retain = FALSE;

  if (list == NULL || num == 0
      || item == list || index > fz_len (list))
    return err;
  else if (list->insert == NULL)
    return -ENOSYS; /* Not implemented.  */

  if (~list->flags & LISTOPT_PTRS)
    err = list->insert (list, index, num, item);
  else if (item != NULL)
    {
      if ((list->flags & LISTOPT_KEEP) == LISTOPT_KEEP
          || ((list->flags & LISTOPT_PASS) == LISTOPT_PASS
              && fz_index_of (list, item, fz_cmp_ptr) >= 0))
        can_retain = TRUE;

      err = list->insert (list, index, num, &item);
      if (can_retain == TRUE && err >= 0)
        fz_retain (item);
    }

  return err;
}

/* Erase implementation wrapper.  */
int_t
fz_erase (list_t *list, uint_t index, uint_t num)
{
  if (list == NULL || index + num > fz_len (list))
    return -EINVAL;
  else if (num == 0)
    return (int_t) index;
  else if (list->erase == NULL)
    return -ENOSYS; /* Not implemented.  */

  if ((list->flags & LISTOPT_KEEP) == LISTOPT_KEEP
      || (list->flags & LISTOPT_PASS) == LISTOPT_PASS)
    fz_del (fz_at (list, index));

  return list->erase (list, index, num);
}

/* Clear LIST and allocate space for SIZE new items.  */
int_t
fz_clear (list_t *list, size_t size)
{
  int_t err = fz_erase (list, 0, fz_len (list));
  if (err < 0)
    return err;

  if (size == 0)
    return 0;

  err = fz_insert (list, 0, size, NULL);
  if (err < 0)
    return err;

  return (int_t) size;
}

/* Find the first occurance of ITEM in LIST.  */
int_t
fz_index_of (list_t *list, const ptr_t item, cmp_f compare)
{
  uint_t i;
  size_t len;

  if (list == NULL)
    return -EINVAL;

  if (compare == NULL)
    compare = fz_cmp_ptr;

  len = fz_len (list);
  for (i = 0; i < len; ++i)
    if (compare (fz_at (list, i), item) == 0)
      return i;

  return -1;
}

/* Comparator functions for search and sort.  */
int_t
fz_cmp_ptr (const ptr_t a, const ptr_t b)
{
  if (a == b)
    return 0;
  else if (a == NULL)
    return 1;
  return -1;
}

int_t
fz_cmp_int (const ptr_t a, const ptr_t b)
{
  return *((int_t *) a) - *((int_t *) b);
}

int_t
fz_cmp_real (const ptr_t a, const ptr_t b)
{
  return *((real_t *) a) - *((real_t *) b);
}

/* Concrete random access list class struct.  */
typedef struct
{
  list_t __parent;
  ptr_t items;
  size_t length;
} vector_t;

/* `fz_len' implementation for `vector_c'.  */
static size_t
vector_length (const ptr_t list)
{
  /* `fz_len' makes a NULL pointer check.  */
  return ((const vector_t *) list)->length;
}

/* Class `vector_c' implementation of `fz_at'.  */
static ptr_t
vector_at (const list_t *list, uint_t index)
{
  vector_t *self = (vector_t *) list;
  return self->items + (index * list->type_size);
}

/* Class `vector_c' implementation of `fz_insert'.  */
static int_t
vector_insert (list_t *list, uint_t index, uint_t num, ptr_t item)
{
  vector_t *self = (vector_t *) list;
  size_t length = self->length + num;
  /* We rely on the general allocation growth policy.  */
  self->items = fz_realloc (self->items, length * list->type_size);

  if (index != self->length)
    {
      memmove (self->items + ((index + num) * list->type_size),
               self->items + (index * list->type_size),
               (self->length - index) * list->type_size);
    }

  if (item == NULL)
    memset (self->items + (index * list->type_size),
            0,
            list->type_size * num);
  else
    memcpy (self->items + (index * list->type_size),
            item,
            list->type_size * num);

  self->length = length;

  return index;
}

/* Class `vector_c' implementation of `fz_erase'.  */
static int_t
vector_erase (list_t *list, uint_t index, uint_t num)
{
  vector_t *self = (vector_t *) list;
  self->length -= num;

  if (index != self->length)
    {
      memmove (self->items + (index * list->type_size),
               self->items + ((index + num) * list->type_size),
               (self->length - index) * list->type_size);
    }

  return index;
}

/* Concrete constructor for `vector_c'.  */
static ptr_t
vector_constructor (ptr_t ptr, va_list *args)
{
  vector_t *self = (vector_t *) list_constructor (ptr, args);
  self->items = NULL;
  self->length = 0;
  list_t *parent = (list_t *) self;
  parent->insert = vector_insert;
  parent->erase = vector_erase;
  parent->at = vector_at;
  return self;
}

/* Concrete destructor for `vector_c'.  */
static ptr_t
vector_destructor (ptr_t ptr)
{
  vector_t *self = (vector_t *) list_destructor (ptr);
  if (self->items != NULL)
    fz_free (self->items);
  return self;
}

/* `vector_c' class descriptor.  */
static const class_t _vector_c = {
  sizeof (vector_t),
  vector_constructor,
  vector_destructor,
  vector_length,
  NULL,
  NULL
};

const class_t *vector_c = &_vector_c;
