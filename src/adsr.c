/* ADSR modulator interface implementation.
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

#include <errno.h>
#include "adsr.h"
#include "mod.h"
#include "private-mod.h"

/* Struct to keep track of individual voice states.  */
struct state_s {
  unsigned char state;
  real_t pos;
  real_t ra;
};

/* ADSR class struct.  */
typedef struct adsr_s
{
  mod_t __parent;
  real_t al; /* Attack length     */
  real_t aa; /* Attack amplitude  */
  real_t dl; /* Decay length      */
  real_t da; /* Decay amplitude   */
  real_t sl; /* Sustain length    */
  real_t sa; /* Sustain amplitude */
  real_t rl; /* Release length    */
} adsr_t;

/* `fz_mod_render' callback.  */
static int_t
adsr_render (mod_t *mod, const request_t *request)
{
  adsr_t *self = (adsr_t *) mod;
  bool_t pressed;
  real_t pressure;
  real_t *steps;
  uint_t i;
  size_t nrendered;
  struct state_s *state = fz_mod_state (mod, request->voice,
                                        struct state_s);

  if (state == NULL || request->srate <= 0)
    return -EINVAL;

  pressed = fz_voice_pressed (request->voice);
  pressure = fz_voice_pressure (request->voice);

  if (pressed == TRUE && state->state == ADSR_STATE_SILENT)
    {
      state->state = ADSR_STATE_ATTACK;
      state->pos = 0;
    }
  else if (pressed == FALSE && state != ADSR_STATE_SILENT)
    {
      state->state = ADSR_STATE_RELEASE;
      state->pos = 0;
    }

  nrendered = fz_len (mod->stepbuf);
  steps = (real_t *) fz_list_data (mod->stepbuf);

  for (i = 0; i < nrendered; ++i)
    {
      switch (state->state)
        {
        case ADSR_STATE_ATTACK:

          if (state->pos < self->al)
            {
              steps[i] = pressure
                * (state->pos * (self->aa / self->al));
              break;
            }

          state->state = ADSR_STATE_DECAY;
          while (state->pos > self->al && state->pos > 0)
            state->pos -= self->al;

        case ADSR_STATE_DECAY:

          if (state->pos < self->dl)
            {
              steps[i] = pressure
                * (self->aa +
                   (state->pos * ((self->da - self->aa) / self->dl)));
              break;
            }

          state->state = ADSR_STATE_SUSTAIN;
          while (state->pos > self->dl && state->pos > 0)
            state->pos -= self->dl;

        case ADSR_STATE_SUSTAIN:

          if (state->pos < self->sl)
            steps[i] = pressure
              * (self->da +
                 (state->pos * ((self->sa - self->da) / self->sl)));
          else
            steps[i] = pressure * self->sa;
          break;

        case ADSR_STATE_RELEASE:

          if (state->pos < self->rl)
            {
              steps[i] = (self->rl - state->pos)
                * (state->ra / self->rl);
              break;
            }
          state->state = ADSR_STATE_SILENT;

        default: /* ADSR_STATE_SILENT */
          steps[i] = 0;
          break;
        }

      state->pos += 1. / request->srate;
      if (state->state != ADSR_STATE_RELEASE)
        state->ra = steps[i]; /* Remeber starting point for release */
    }

  return nrendered;
}

/* ADSR constructor.  */
static ptr_t
adsr_constructor (ptr_t ptr, va_list *args)
{
  adsr_t *self = (adsr_t *)
    ((const class_t *) mod_c)->construct (ptr, args);
  self->__parent.render = adsr_render;
  self->al = 0.10;
  self->aa = 1.00;
  self->dl = 0.10;
  self->da = 0.50;
  self->sl = 0.20;
  self->sa = 0.75;
  self->rl = 0.40;
  return self;
}

/* ADSR destructor.  */
static ptr_t
adsr_destructor (ptr_t ptr)
{
  adsr_t *self = (adsr_t *)
    ((const class_t *) mod_c)->destruct (ptr);
  return self;
}

/* ADSR class descriptor.  */
static const class_t _adsr_c = {
  sizeof (adsr_t),
  adsr_constructor,
  adsr_destructor,
  NULL,
  NULL,
  NULL
};

const class_t *adsr_c = &_adsr_c;
