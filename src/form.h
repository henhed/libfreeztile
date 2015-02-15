/* Header file exposing form node interface.
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

#ifndef FZ_FORM_H
#define FZ_FORM_H 1

#include "class.h"

__BEGIN_DECLS

#define SHAPE_SINE 0
#define SHAPE_TRIANGLE 1
#define SHAPE_SQUARE 2
#define FORM_SLOT_FREQ 1
#define FORM_SLOT_AMP 2

typedef struct form_s form_t;

extern int_t fz_form_set_shape (form_t *, int_t);
extern real_t fz_form_get_shifting (const form_t *);
extern uint_t fz_form_set_shifting (form_t *, real_t);
extern real_t fz_form_get_portamento (const form_t *);
extern uint_t fz_form_set_portamento (form_t *, real_t);
extern real_t fz_form_get_pitch (const form_t *);
extern uint_t fz_form_set_pitch (form_t *, real_t);

extern const class_t *form_c;

__END_DECLS

#endif /* ! FZ_FORM_H */
