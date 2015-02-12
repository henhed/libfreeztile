/* Header file defining LFO modulator interface.
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

#ifndef FZ_LFO_H
#define FZ_LFO_H 1

#include "class.h"

__BEGIN_DECLS

typedef struct lfo_s lfo_t;

extern real_t fz_lfo_get_frequency (lfo_t *);
extern uint_t fz_lfo_set_frequency (lfo_t *, real_t);
extern int_t fz_lfo_set_shape (lfo_t *, int_t);

extern const class_t *lfo_c;

__END_DECLS

#endif /* ! FZ_LFO_H */
