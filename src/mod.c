/* Implementation of modulator class interface.
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
#include "mod.h"
#include "private-mod.h"
#include "class.h"
#include "list.h"
#include "malloc.h"

#define MOD_NONE 0
#define MOD_RENDERED (1 << 0)

struct voice_state_s {
  voice_t *voice;
  ptr_t data;
};

/* Modulator constructor.  */
static ptr_t
mod_constructor (ptr_t ptr, va_list *args)
{
  mod_t *self = (mod_t *) ptr;
  self->stepbuf = fz_new_simple_vector (real_t);
  self->modbuf = fz_new_simple_vector (real_t);
  self->vstates = fz_new_simple_vector (struct voice_state_s);
  self->flags = MOD_RENDERED;
  self->render = NULL;
  self->freestate = NULL;
  return self;
}

/* Modulator destructor.  */
static ptr_t
mod_destructor (ptr_t ptr)
{
  mod_t *self = (mod_t *) ptr;
  uint_t i;
  size_t nvstates = fz_len (self->vstates);
  struct voice_state_s *state;

  for (i = 0; i < nvstates; ++i)
    {
      state = fz_ref_at (self->vstates, i, struct voice_state_s);
      if (self->freestate != NULL)
        self->freestate (self, state->data);
      fz_free (state->data);
    }

  fz_del (self->vstates);
  fz_del (self->modbuf);
  fz_del (self->stepbuf);
  return self;
}

/* Get state data for the given MODULATOR and VOICE. If no data
   exists, SIZE bytes are allocetd and returned.  */
ptr_t
fz_mod_state_data (mod_t *modulator, voice_t *voice, size_t size)
{
  int_t i;
  size_t nstates;
  struct voice_state_s *state;
  struct voice_state_s newstate;

  if (modulator == NULL || voice == NULL)
    return NULL;

  nstates = fz_len (modulator->vstates);
  for (i = 0; i < nstates; ++i)
    {
      state = fz_ref_at (modulator->vstates, i, struct voice_state_s);
      if (state->voice == voice)
        return state->data;
    }

  if (size == 0)
    return NULL;

  newstate.voice = voice;
  newstate.data = fz_malloc (size);

  i = fz_push_one (modulator->vstates, &newstate);
  if (i >= 0)
    return fz_ref_at (modulator->vstates, i,
                      struct voice_state_s)->data;

  return NULL;
}

/* Prepare SELF for `fz_mod_render' to render NFRAMES new frames.  */
void
fz_mod_prepare (mod_t *self, size_t nframes)
{
  if (self == NULL)
    return;

  self->flags &= ~MOD_RENDERED;
  fz_clear (self->stepbuf, nframes);
  fz_clear (self->modbuf, nframes);
}

/* Render NFRAMES of node modulation input into MOD buffer.  */
int_t
fz_mod_render (mod_t *self, const request_t *request)
{
  size_t nframes;

  if (self == NULL)
    return -EINVAL;

  nframes = fz_len (self->stepbuf);

  if (self->flags & MOD_RENDERED)
    return nframes;

  if (request == NULL)
    return -EINVAL;

  self->flags |= MOD_RENDERED;
  if (self->render != NULL && nframes != 0)
    return self->render (self, request);

  return nframes;
}

/* Apply modulation from rendered SELF on OUT buffer using LO and UP
   as lower and upper limit.  */
int_t
fz_mod_apply (const mod_t *self, list_t *out, real_t lo, real_t up)
{
  real_t *moddata;
  real_t *outdata;
  size_t modsize;
  size_t outsize;
  size_t napplied = 0;
  real_t range = up - lo;
  uint_t i;

  if (self == NULL || out == NULL)
    return -EINVAL;

  modsize = fz_len (self->stepbuf);
  outsize = fz_len (out);
  moddata = (real_t *) fz_list_data (self->stepbuf);
  outdata = (real_t *) fz_list_data (out);
  if (outdata == NULL)
    return -EINVAL; /* OUT is not a vector.  */

  napplied = (modsize < outsize ? modsize : outsize);
  for (i = 0; i < napplied; ++i)
    outdata[i] *= ((moddata[i] * range) + lo);

  return napplied;
}

/* Return a buffer whith values modulated around the given SEED.  */
const list_t *
fz_modulate (const mod_t *self, real_t seed, real_t lo, real_t up)
{
  real_t *moddata;
  size_t modsize;
  uint_t i;

  if (self == NULL)
    return NULL;

  fz_clear (self->modbuf, fz_len (self->stepbuf));
  moddata = (real_t *) fz_list_data (self->modbuf);
  modsize = fz_len (self->modbuf);

  for (i = 0; i < modsize; ++i)
    moddata[i] = seed;

  if (fz_mod_apply (self, self->modbuf, lo, up) < 0)
    return NULL;

  return self->modbuf;
}

/* `mod_c' class descriptor.  */
static const class_t _mod_c = {
  sizeof (mod_t),
  mod_constructor,
  mod_destructor,
  NULL,
  NULL,
  NULL
};

const class_t *mod_c = &_mod_c;
