/* Header file declaring modulator class interface.
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

#ifndef FZ_MOD_H
#define FZ_MOD_H 1

#include "class.h"
#include "voice.h"
#include "list.h"

__BEGIN_DECLS

#define fz_modulate_snorm(mod, seed) \
  fz_modulate (mod, seed, -1,  1)

#define fz_modulate_unorm(mod, seed) \
  fz_modulate (mod, seed, 0,  1)

typedef struct mod_s mod_t;

extern int_t fz_mod_render (mod_t *, size_t, const request_t *);
extern int_t fz_mod_apply (const mod_t *, list_t *, real_t, real_t);
extern const list_t * fz_modulate (const mod_t *, real_t, real_t, real_t);

extern const class_t *mod_c;

__END_DECLS

#endif /* ! FZ_MOD_H */
