/* Header file for class management functions.
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

#ifndef FZ_CLASS_H
#define FZ_CLASS_H 1

#include <stdarg.h>
#include "defs.h"

__BEGIN_DECLS

/* Class descriptor struct.  */
typedef struct
{
  size_t size;
  ptr_t (*construct) (ptr_t, va_list *);
  ptr_t (*destruct) (ptr_t);
  size_t (*length) (const ptr_t);
  ptr_t (*clone) (const ptr_t, ptr_t);
  int_t (*compare) (const ptr_t, const ptr_t);
} class_t;

extern ptr_t fz_new (const class_t *, ...);
extern int_t fz_del (ptr_t);
extern size_t fz_len (const ptr_t);
extern ptr_t fz_clone (const ptr_t);
extern int_t fz_cmp (const ptr_t, const ptr_t);

__END_DECLS

#endif /* ! FZ_CLASS_H */
