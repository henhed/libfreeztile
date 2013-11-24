/* Implementation of list functions.
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
  int_t (*insert) (list_t *, uint_t, uint_t, ptr_t);
  int_t (*erase) (list_t *, uint_t, uint_t);
  ptr_t (*at) (list_t *, uint_t);
};

/* Abstract list constuctor.  */
static ptr_t
list_constructor (ptr_t ptr, va_list *args)
{
  list_t *self = (list_t *) ptr;
  size_t type_size = va_arg (*args, size_t);
  const char *type_name = va_arg (*args, char *);

  self->type_name = (char *) fz_malloc (sizeof (char) *
					(strlen (type_name) + 1));
  strcpy (self->type_name, type_name);
  self->type_size = type_size;
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
  fz_free (self->type_name);
  return self;
}

/* Abstract list item getter.  */
ptr_t
fz_at (list_t *list, uint_t index)
{
  if (list == NULL || list->at == NULL || index >= fz_len (list))
    return NULL;

  return list->at (list, index);
}

/* Insert concrete implementation wrapper.  */
int_t
fz_insert (list_t *list, uint_t index, uint_t num, ptr_t item)
{
  if (list == NULL || num == 0 || item == NULL || index > fz_len (list))
    return -EINVAL;
  else if (list->insert == NULL)
    return -ENOSYS; /* Not implemented.  */

  return list->insert (list, index, num, item);
}

/* Erase implementation wrapper.  */
int_t
fz_erase (list_t *list, uint_t index, uint_t num)
{
  if (list == NULL || num == 0 || index + num > fz_len (list))
    return -EINVAL;
  else if (list->erase == NULL)
    return -ENOSYS; /* Not implemented.  */

  return list->erase (list, index, num);
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
vector_at (list_t *list, uint_t index)
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

  memcpy (self->items + (index * list->type_size), item, list->type_size * num);
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
