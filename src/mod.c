/* Implementation of modulator class interface.
   Copyright (C) 2013 Henrik Hedelund.

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

#include "mod.h"
#include "private-mod.h"
#include "class.h"
#include "list.h"
#include "malloc.h"

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
  self->vstates = fz_new_simple_vector (struct voice_state_s);
  return self;
}

/* Modulator destructor.  */
static ptr_t
mod_destructor (ptr_t ptr)
{
  mod_t *self = (mod_t *) ptr;
  uint_t i;
  size_t nvstates = fz_len (self->vstates);

  for (i = 0; i < nvstates; ++i)
    fz_free (fz_ref_at (self->vstates, i,
                        struct voice_state_s)->data);

  fz_del (self->vstates);
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

/* Render node modulation input inte MODULATOR buffer.  */
int_t
fz_mod_render (mod_t *modulator, const request_t *request)
{
  return 0;
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
