/* Memory allocation related functions.
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

#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include "malloc.h"

/* Keeps track of memory usage for debugging purposes.  */
static uint_t _total_meta = 0;
static uint_t _total_disp = 0;

/* A `memory_meta' struct is embedded in each allocation.  */
struct memory_meta
{
  uint_t numref;
  size_t len;
};

/* Reference counting and memory usage tracking  version of `realloc'.
   The meta is placed in memory before the returned pointer.  */
ptr_t
fz_realloc (ptr_t ptr, size_t len)
{
  struct memory_meta *meta;

  if (ptr != NULL)
    {
      /* If a pointer is given we expect to find an embedded meta struct.  */
      meta = ((struct memory_meta *) ptr) - 1;
      if (meta->len >= len)
	/* Current size is sufficent.  */
	return ptr;
      else
	{
	  /* Not sufficent, decrease global counter.  */
	  _total_disp -= meta->len;
	}
    }
  else
    meta = NULL;

  /* Allocate new space and increase global counter.  */
  meta = ((struct memory_meta *)
	  realloc (meta, sizeof (struct memory_meta) + len));
  assert (meta);
  _total_disp += (meta->len = len);

  if (ptr == NULL)
    {
      /* Reset memory and increase meta counter if PTR is new.  */
      _total_meta += sizeof (struct memory_meta);
      meta->numref = 1;
      memset (meta + 1, 0, len);
    }

  return meta + 1;
}

/* Refenece counting version of `malloc'. See `fz_realloc'.  */
ptr_t
fz_malloc (size_t len)
{
  return fz_realloc (NULL, len);
}

/* Increase the reference counter of pointer PTR.  */
ptr_t
fz_retain (ptr_t ptr)
{
  if (ptr != NULL)
    ++(((struct memory_meta *) ptr) - 1)->numref;
  return ptr;
}

/* Free the memory pointed to by PTR if decreasing it's reference
   counter makes it reach zero.  Returns the number of references
   remaining after a decrease or a negative error code on error.  */
int_t
fz_free (ptr_t ptr)
{
  if (ptr == NULL)
    return -EINVAL;

  struct memory_meta *meta = ((struct memory_meta *) ptr) - 1;

  if (--(meta->numref) == 0)
    {
      /* Decrease memory usage counters and free memory if
	 reference counter reaches zero.  */
      _total_meta -= sizeof (struct memory_meta);
      _total_disp -= meta->len;
      free (meta);
      return 0;
    }

  return meta->numref;
}

/* Check the current fz_* function family memory usage.  */
size_t
fz_memusage (uint_t flags)
{
  size_t size = 0;
  if ((flags & MEMUSAGE_META) || flags == 0)
    size += _total_meta;
  if ((flags & MEMUSAGE_DISP) || flags == 0)
    size += _total_disp;
  return size;
}
