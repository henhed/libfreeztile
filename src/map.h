/* Header file for map functions.
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

#ifndef FZ_MAP_H
#define FZ_MAP_H 1

#include "class.h"
#include "list.h"

__BEGIN_DECLS

#define fz_map_each(map, var)                               \
  for ((var) = fz_map_next ((map), NULL);                   \
       (var) != NULL;                                       \
       (var) = fz_map_next ((map), (const ptr_t) (var)))

typedef struct map_s map_t;
typedef void (*map_value_f) (const map_t *, uintptr_t, ptr_t);

extern ptr_t fz_map_owner (const map_t *);
extern ptr_t fz_map_get (const map_t *, uintptr_t);
extern ptr_t fz_map_set (map_t *, uintptr_t, const ptr_t, size_t);
extern void fz_map_unset (map_t *, uintptr_t);
extern uintptr_t fz_map_key (const ptr_t);
extern ptr_t fz_map_next (const map_t *, const ptr_t);

extern const class_t *map_c;

__END_DECLS

#endif /* ! FZ_MAP_H */
