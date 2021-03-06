/* Header file declaring voice class interface.
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

#ifndef FZ_VOICE_H
#define FZ_VOICE_H 1

#include "class.h"
#include "list.h"

__BEGIN_DECLS

#define A4_ID 69
#define A4_FREQ 440.0
#define TWELFTH_ROOT_OF_TWO 1.05946309435929526

#ifndef DEFAULT_SAMPLE_RATE
# define DEFAULT_SAMPLE_RATE 44100
#endif

#define VOICE_POOL_PRIORITY_FIFO 0
#define VOICE_POOL_PRIORITY_PRESSURE 1

typedef struct voice_s voice_t;
typedef struct vpool_s vpool_t;

extern real_t fz_get_sample_rate ();
extern int_t fz_set_sample_rate (real_t);

extern int_t fz_voice_press (voice_t *, real_t, real_t);
extern int_t fz_voice_aftertouch (voice_t *, real_t);
extern int_t fz_voice_release (voice_t *);
extern bool_t fz_voice_pressed (const voice_t *);
extern real_t fz_voice_frequency (const voice_t *);
extern real_t fz_voice_velocity (const voice_t *);
extern real_t fz_voice_pressure (const voice_t *);
extern bool_t fz_voice_repossessed (const voice_t *);

extern int_t fz_vpool_press (vpool_t *, uint_t, real_t);
extern int_t fz_vpool_release (vpool_t *, uint_t);
extern int_t fz_vpool_kill (vpool_t *, voice_t *);
extern int_t fz_vpool_kill_id (vpool_t *, uint_t);
extern const list_t * fz_vpool_voices (vpool_t *);

extern real_t fz_note_frequency (const char *);

extern const class_t *voice_c;
extern const class_t *vpool_c;

__END_DECLS

#endif /* ! FZ_VOICE_H */
