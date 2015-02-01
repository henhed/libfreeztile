/* Implementation of voice class interface.
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

#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include "voice.h"
#include "malloc.h"
#include "class.h"
#include "list.h"

/* voice class struct.  */
struct voice_s
{
  const class_t *__class;
  uint_t id;
  bool_t pressed;
  real_t frequency;
  real_t velocity;
  real_t pressure;
};

/* voice pool class struct.  */
struct vpool_s
{
  const class_t *__class;
  list_t *pool;
  list_t *active_voices;
};

/* Voice constructor.  */
static ptr_t
voice_constructor (ptr_t ptr, va_list *args)
{
  voice_t *self = (voice_t *) ptr;
  self->id = 0;
  self->pressed = FALSE;
  self->frequency = 440;
  self->velocity = 0;
  self->pressure = self->velocity;
  return self;
}

/* 'Press' VOICE with a given FREQUENCY and VELOCITY.  */
int_t
fz_voice_press (voice_t *voice, real_t frequency, real_t velocity)
{
  if (voice == NULL
      || voice->pressed == TRUE
      || frequency <= 0
      || (velocity <= 0 || velocity > 1))
    return EINVAL;

  voice->pressed = TRUE;
  voice->frequency = frequency;
  voice->pressure = voice->velocity = velocity;

  return 0;
}

/* 'Touch' a pressed VOICE with a given amount of PRESSURE.  */
int_t
fz_voice_aftertouch (voice_t *voice, real_t pressure)
{
  if (voice == NULL
      || voice->pressed == FALSE
      || (pressure <= 0 || pressure > 1))
    return EINVAL;

  voice->pressure = pressure;

  return 0;
}

/* 'Release' a pressed VOICE.  */
int_t
fz_voice_release (voice_t *voice)
{
  if (voice == NULL || voice->pressed == FALSE)
    return EINVAL;

  voice->pressed = FALSE;
  voice->pressure = voice->velocity = 0;

  return 0;
}

/* Report the state of VOICE.  */
bool_t
fz_voice_pressed (const voice_t *voice)
{
  return voice == NULL ? FALSE : voice->pressed;
}

/* Report the current frequency of VOICE.  */
real_t
fz_voice_frequency (const voice_t *voice)
{
  return voice == NULL ? 0 : voice->frequency;
}

/* Report the press-time velocity of VOICE.  */
real_t
fz_voice_velocity (const voice_t *voice)
{
  return voice == NULL ? 0 : voice->velocity;
}

/* Report the current pressure of VOICE.  */
real_t
fz_voice_pressure (const voice_t *voice)
{
  return voice == NULL ? 0 : voice->pressure;
}

/* Voice pool constructor.  */
static ptr_t
vpool_constructor (ptr_t ptr, va_list *args)
{
  vpool_t *self = (vpool_t *) ptr;
  size_t polyphony = va_arg (*args, size_t);
  self->pool = fz_new_owning_vector (voice_t *);
  self->active_voices = fz_new_owning_vector (voice_t *);
  for (; polyphony > 0; --polyphony)
    fz_push_one (self->pool, fz_new (voice_c));
  /* Pre-allocate space for active voices.  */
  fz_clear (self->active_voices, fz_len (self->pool));
  fz_clear (self->active_voices, 0);
  return self;
}

/* Voice pool destructor.  */
static ptr_t
vpool_destructor (ptr_t ptr)
{
  vpool_t *self = (vpool_t *) ptr;
  fz_del (self->active_voices);
  fz_del (self->pool);
  return self;
}

/* Get active voice from given POOL by ID.  */
static voice_t *
vpool_get_active_voice (const vpool_t *pool, uint_t id)
{
  if (pool == NULL)
    return NULL;

  uint_t i;
  voice_t *voice = NULL;
  size_t nvoices = fz_len (pool->active_voices);
  for (i = 0; i < nvoices; ++i)
    {
      voice = fz_ref_at (pool->active_voices, i, voice_t);
      if (voice->id == id)
        return voice;
    }

  return NULL;
}

/* Press a voice from POOL.  */
int_t
fz_vpool_press (vpool_t *pool, uint_t id, real_t velocity)
{
  if (pool == NULL)
    return EINVAL;

  size_t poolsize;
  size_t nactive;
  voice_t *voice = vpool_get_active_voice (pool, id);
  real_t frequency = A4_FREQ * pow (TWELFTH_ROOT_OF_TWO,
                                    ((int_t) id) - A4_ID);

  if (voice != NULL)
    {
      fz_voice_release (voice);
      return fz_voice_press (voice, frequency, velocity);
    }

  poolsize = fz_len (pool->pool);
  nactive = fz_len (pool->active_voices);

  if (poolsize + nactive == 0)
    return 0;

  if (poolsize > 0)
    {
      voice = fz_ref_at (pool->pool, poolsize - 1, voice_t);
      fz_retain (voice);
      fz_erase_one (pool->pool, poolsize - 1);
    }
  else if (nactive > 0)
    {
      /* Steal oldest voice (FIFO).  */
      voice = fz_ref_at (pool->active_voices, 0, voice_t);
      fz_retain (voice);
      fz_erase_one (pool->active_voices, 0);
      fz_voice_release (voice);
    }

  voice->id = id;
  fz_push_one (pool->active_voices, voice);
  return fz_voice_press (voice, frequency, velocity);
}

/* Relase voice with given ID in POOL.  */
int_t
fz_vpool_release (vpool_t *pool, uint_t id)
{
  if (pool == NULL)
    return EINVAL;

  voice_t *voice = vpool_get_active_voice (pool, id);
  if (voice == NULL)
    return EINVAL;

  return fz_voice_release (voice);
}

/* Get active voices from POOL.  */
const list_t *
fz_vpool_voices (vpool_t *pool)
{
  return pool == NULL ? NULL : pool->active_voices;
}

/* Interpret a string represented NOTE as a Hz frequency.  */
real_t
fz_note_frequency (const char *note)
{
  static const char *names = "c\0d\0ef\0g\0a\0b";
  size_t length;
  uint_t i;
  uint_t pos = 0;
  int_t octave = 4;
  int_t offset;
  char name;

  if (note == NULL)
    return 0;

  length = strlen (note);
  if (length == 0)
    return 0;

  for (; pos < length && isspace (note[pos]); ++pos);

  if (pos == length)
    return 0;

  name = tolower (note[pos]);
  for (i = 0; i < 12; ++i)
    {
      if (name == names[i])
        {
          offset = i - 9;
          break;
        }
      else if (i == 11)
        return 0;
    }

  for (++pos; pos < length; ++pos)
    {
      if (tolower (note[pos]) == 'b')
        --offset;
      else if (note[pos] == '#')
        ++offset;
      else if (isspace (note[pos]))
        continue;
      else
        break; /* Octave inicator or junk.  */
    }

  octave = (pos < length ? atoi (note + pos) : octave) - 4;
  return 440. * pow (TWELFTH_ROOT_OF_TWO, offset + (12 * octave));
}

/* `voice_c' class descriptor.  */
static const class_t _voice_c = {
  sizeof (voice_t),
  voice_constructor,
  NULL,
  NULL,
  NULL,
  NULL
};

/* `vpool_c' class descriptor.  */
static const class_t _vpool_c = {
  sizeof (vpool_t),
  vpool_constructor,
  vpool_destructor,
  NULL,
  NULL,
  NULL
};

const class_t *voice_c = &_voice_c;
const class_t *vpool_c = &_vpool_c;
