/* Header file exposing delay node interface.
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

#ifndef FZ_DELAY_H
#define FZ_DELAY_H 1

#include "class.h"

__BEGIN_DECLS

#ifndef DELAY_TIME_MAX
# define DELAY_TIME_MAX 3
#endif

typedef struct delay_s delay_t;

extern real_t fz_delay_get_gain (const delay_t *);
extern int_t fz_delay_set_gain (delay_t *, real_t);
extern real_t fz_delay_get_feedback (const delay_t *);
extern int_t fz_delay_set_feedback (delay_t *, real_t);
extern real_t fz_delay_get_delay (const delay_t *);
extern int_t fz_delay_set_delay (delay_t *, real_t);

extern const class_t *delay_c;

__END_DECLS

#endif /* ! FZ_DELAY_H */
