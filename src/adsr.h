/* Header file defining ADSR modulator interface.
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

#ifndef FZ_ADSR_H
#define FZ_ADSR_H 1

#include "class.h"
#include "voice.h"

__BEGIN_DECLS

#define ADSR_STATE_SILENT 0
#define ADSR_STATE_ATTACK 1
#define ADSR_STATE_DECAY 2
#define ADSR_STATE_SUSTAIN 3
#define ADSR_STATE_RELEASE 4

#define fz_adsr_is_silent(adsr, voice) \
  (fz_adsr_get_state ((adsr), (voice)) == ADSR_STATE_SILENT \
   ? TRUE : FALSE)

typedef struct adsr_s adsr_t;

extern int_t fz_adsr_get_state (const adsr_t *, const voice_t *);
extern real_t fz_adsr_get_a_len (const adsr_t *);
extern uint_t fz_adsr_set_a_len (adsr_t *, real_t);
extern real_t fz_adsr_get_a_amp (const adsr_t *);
extern uint_t fz_adsr_set_a_amp (adsr_t *, real_t);
extern real_t fz_adsr_get_d_len (const adsr_t *);
extern uint_t fz_adsr_set_d_len (adsr_t *, real_t);
extern real_t fz_adsr_get_d_amp (const adsr_t *);
extern uint_t fz_adsr_set_d_amp (adsr_t *, real_t);
extern real_t fz_adsr_get_s_len (const adsr_t *);
extern uint_t fz_adsr_set_s_len (adsr_t *, real_t);
extern real_t fz_adsr_get_s_amp (const adsr_t *);
extern uint_t fz_adsr_set_s_amp (adsr_t *, real_t);
extern real_t fz_adsr_get_r_len (const adsr_t *);
extern uint_t fz_adsr_set_r_len (adsr_t *, real_t);

extern const class_t *adsr_c;

__END_DECLS

#endif /* ! FZ_ADSR_H */
