/* Header file exposing filter node interface.
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

#ifndef FZ_FILTER_H
#define FZ_FILTER_H 1

#include "class.h"

__BEGIN_DECLS

#define FILTER_TYPE_LOWPASS 0
#define FILTER_TYPE_HIGHPASS 1
#define FILTER_TYPE_BANDPASS 2

typedef struct filter_s filter_t;

extern int_t fz_filter_get_type (const filter_t *);
extern int_t fz_filter_set_type (filter_t *, int_t);
extern real_t fz_filter_get_frequency (const filter_t *);
extern int_t fz_filter_set_frequency (filter_t *, real_t);
extern real_t fz_filter_get_resonance (const filter_t *);
extern int_t fz_filter_set_resonance (filter_t *, real_t);

extern const class_t *filter_c;

__END_DECLS

#endif /* ! FZ_FILTER_H */
