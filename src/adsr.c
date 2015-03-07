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

#include "adsr.h"
#include "mod.h"
#include "private-mod.h"
#include "defs.h"

/* Struct to keep track of individual voice states.  */
struct state_s {
  unsigned char state;
  real_t pos;
  real_t ra; /* Release amplitude  */
  real_t pa; /* Previous amplitude */
  real_t freq;
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
  real_t freq;
  real_t *steps;
  uint_t i;
  size_t nrendered;
  real_t pa, aa, da, sa, ra;
  real_t aslope, dslope, sslope, rslope;
  struct state_s *state = fz_mod_state (mod, request->voice,
                                        struct state_s);

  if (state == NULL || request->srate <= 0)
    return -EINVAL;

  pressed = fz_voice_pressed (request->voice);
  pressure = fz_voice_pressure (request->voice);
  freq = fz_voice_frequency (request->voice);

  pa = state->pa;
  aa = self->aa * pressure;
  da = self->da * pressure;
  sa = self->sa * pressure;
  ra = state->ra;
  /* The conditional statements here are just a division-by-zero
     prevention. If a parts length is <= 0, the (0) slope will never
     be used.  */
  aslope = self->al > 0 ? (aa - pa) / self->al : 0;
  dslope = self->dl > 0 ? (da - aa) / self->dl : 0;
  sslope = self->sl > 0 ? (sa - da) / self->sl : 0;
  rslope = self->rl > 0 ? ra / self->rl : 0;

  if (pressed == TRUE
      && (state->state == ADSR_STATE_SILENT
          || state->state == ADSR_STATE_RELEASE
          || state->freq != freq))
    {
      state->state = ADSR_STATE_ATTACK;
      state->pos = 0;
      state->freq = freq;
    }
  else if (pressed == FALSE
           && state->state != ADSR_STATE_SILENT
           && state->state != ADSR_STATE_RELEASE)
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
              steps[i] = self->al <= 0
                ? aa
                : pa + (state->pos * aslope);
              break;
            }

          state->state = ADSR_STATE_DECAY;
          while (state->pos > self->al && state->pos > 0
                 && self->al > 0)
            state->pos -= self->al;

        case ADSR_STATE_DECAY:

          if (state->pos < self->dl)
            {
              steps[i] = self->dl <= 0
                ? da
                : aa + (state->pos * dslope);
              break;
            }

          state->state = ADSR_STATE_SUSTAIN;
          while (state->pos > self->dl && state->pos > 0
                 && self->dl > 0)
            state->pos -= self->dl;

        case ADSR_STATE_SUSTAIN:

          if (state->pos < self->sl && self->sl > 0)
            steps[i] = da + (state->pos * sslope);
          else
            steps[i] = sa;
          break;

        case ADSR_STATE_RELEASE:

          if (state->pos < self->rl && self->rl > 0)
            {
              steps[i] = (self->rl - state->pos) * rslope;
              break;
            }
          state->state = ADSR_STATE_SILENT;

        default: /* ADSR_STATE_SILENT */
          steps[i] = 0;
          break;
        }

      state->pos += 1. / request->srate;
    }

  if (nrendered > 0)
    {
      if (state->state != ADSR_STATE_ATTACK)
        /* Remeber previous amplitude to reduce clipping on the next
           attack if this envelope is cut off before it reaches the
           silent state.  */
        state->pa = steps[nrendered - 1];

      if (state->state != ADSR_STATE_RELEASE)
        /* Remeber starting point for release.  */
        state->ra = steps[nrendered - 1];
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
  self->al = 0.00;
  self->aa = 1.00;
  self->dl = 0.00;
  self->da = 1.00;
  self->sl = 0.00;
  self->sa = 1.00;
  self->rl = 0.00;
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

/* Get VOICEs current state in ADSR or a negtive error code.  */
int_t
fz_adsr_get_state (const adsr_t *adsr, const voice_t *voice)
{
  if (!adsr || !voice)
    return -EINVAL;

  mod_t *mod = (mod_t *) adsr;
  struct state_s *state = fz_mod_state (mod, voice, struct state_s);
  if (!state)
    return -ENODATA;

  return (int_t) state->state;
}

/* Macro for creating ADSR duration part getter functions.  */
#define CREATE_LEN_GETTER(part, var)            \
  real_t                                        \
  fz_adsr_get_##part##_len (const adsr_t *adsr) \
  {                                             \
    return adsr ? adsr->var : 0;                \
  }

/* Macro for creating ADSR duration part setter functions.  */
#define CREATE_LEN_SETTER(part, var)                     \
  uint_t                                                 \
  fz_adsr_set_##part##_len (adsr_t *adsr, real_t length) \
  {                                                      \
    if (!adsr || length < 0)                             \
      return EINVAL;                                     \
    adsr->var = length;                                  \
    return 0;                                            \
  }

/* Macro for creating ADSR amplitude part getter functions.  */
#define CREATE_AMP_GETTER(part, var)            \
  real_t                                        \
  fz_adsr_get_##part##_amp (const adsr_t *adsr) \
  {                                             \
    return adsr ? adsr->var : 0;                \
  }

/* Macro for creating ADSR amplitude part setter functions.  */
#define CREATE_AMP_SETTER(part, var)                        \
  uint_t                                                    \
  fz_adsr_set_##part##_amp (adsr_t *adsr, real_t amplitude) \
  {                                                         \
    if (!adsr || amplitude < 0 || amplitude > 1)            \
      return EINVAL;                                        \
    adsr->var = amplitude;                                  \
    return 0;                                               \
  }

/* Attack getters / setters.  */
CREATE_LEN_GETTER (a, al)
CREATE_LEN_SETTER (a, al)
CREATE_AMP_GETTER (a, aa)
CREATE_AMP_SETTER (a, aa)

/* Decay getters / setters.  */
CREATE_LEN_GETTER (d, dl)
CREATE_LEN_SETTER (d, dl)
CREATE_AMP_GETTER (d, da)
CREATE_AMP_SETTER (d, da)

/* Sustain getters / setters.  */
CREATE_LEN_GETTER (s, sl)
CREATE_LEN_SETTER (s, sl)
CREATE_AMP_GETTER (s, sa)
CREATE_AMP_SETTER (s, sa)

/* Release getters / setters.  */
CREATE_LEN_GETTER (r, rl)
CREATE_LEN_SETTER (r, rl)

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
