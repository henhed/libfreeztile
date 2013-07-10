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

#include "class.h"

/* Create an instance of the type represented by the TYPE descriptor.  */
ptr_t
fz_new (const class_t *type, ...)
{
  if (type == NULL)
    return NULL;

  ptr_t obj = fz_malloc (type->size);
  if (obj == NULL)
    return NULL;

  if (type->construct)
    {
      va_list ap;
      va_start (ap, type);
      obj = type->construct (obj, &ap);
      va_end (ap);
    }

  return obj;
}
