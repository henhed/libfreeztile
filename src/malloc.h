/* Header file for memory allocation related functions.
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

#ifndef FZ_MALLOC_H
#define FZ_MALLOC_H 1

#include "defs.h"

#define MEMUSAGE_META (1 << 0)
#define MEMUSAGE_DISP (1 << 1)
#define MEMUSAGE_All (MEMUSAGE_META | MEMUSAGE_DISP)

__BEGIN_DECLS

extern ptr_t fz_realloc (ptr_t, size_t);
extern ptr_t fz_malloc (size_t);
extern ptr_t fz_retain (ptr_t);
extern int_t fz_refcount (ptr_t);
extern int_t fz_free (ptr_t);
extern size_t fz_memusage (uint_t flags);

__END_DECLS

#endif /* ! FZ_MALLOC_H */
