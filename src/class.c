/* Implementation of class management functions.
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
#include "class.h"
#include "malloc.h"

/* Create an instance of the type represented by the TYPE descriptor.  */
ptr_t
fz_new (const class_t *type, ...)
{
  if (type == NULL)
    return NULL;

  ptr_t obj = fz_malloc (type->size);
  if (obj == NULL)
    return NULL;

  *((class_t **) obj) = type;
  if (type->construct)
    {
      va_list ap;
      va_start (ap, type);
      obj = type->construct (obj, &ap);
      va_end (ap);
    }

  return obj;
}

/* Delete an object.  */
int_t
fz_del (ptr_t ptr)
{
  if (ptr == NULL)
    return -EINVAL;

  /* Run the destructor if the memory is about to be freed.  */
  if (fz_refcount (ptr) == 1)
    {
      ptr_t (*destructor) (ptr_t) = (*((const class_t **) ptr))->destruct;
      if (destructor != NULL)
        destructor (ptr);
    }

  return fz_free (ptr);
}

/* Measure the length of an object.  */
size_t
fz_len (ptr_t ptr)
{
  if (ptr == NULL)
    return 0;

  size_t (*length) (const ptr_t) = (*((const class_t **) ptr))->length;
  if (length != NULL)
    return length (ptr);

  return 0;
}
