/* Header file for list types functions interface.
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

#ifndef FZ_LIST_H
#define FZ_LIST_H 1

#include "class.h"

__BEGIN_DECLS

typedef struct list_s list_t;

#define fz_at_ref (list, index, type) \
  ((type *) fz_at (list, index))

#define fz_at_val (list, index, type) \
  (*((type *) fz_at (list, index)))

#define fz_insert_one (list, index, item) \
  fz_insert (list, index, 1, item)

#define fz_push (list, num, item) \
  fz_insert (list, fz_len (list), num, item)

#define fz_push_one (list, item) \
  fz_insert (list, fz_len (list), 1, item)

#define fz_erase_one (list, index) \
  fz_erase (list, index, 1)

extern ptr_t fz_at (list_t *, uint_t);
extern int_t fz_insert (list_t *, uint_t, uint_t, ptr_t);
extern int_t fz_erase (list_t *, uint_t, uint_t);

extern const class_t *vector_c;

__END_DECLS

#endif /* ! FZ_LIST_H */
