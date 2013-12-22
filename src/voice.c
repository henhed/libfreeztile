/* Implementation of voice class interface.
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

#include <errno.h>
#include "voice.h"
#include "class.h"

/* voice class struct.  */
struct voice_s
{
  const class_t *__class;
  bool_t pressed;
  real_t frequency;
  real_t velocity;
  real_t pressure;
};

/* Voice constructor.  */
static ptr_t
voice_constructor (ptr_t ptr, va_list *args)
{
  voice_t *self = (voice_t *) ptr;
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

/* Interpret a string represented NOTE as a Hz frequency.  */
real_t
fz_note_frequency (const char *note)
{
  (void) note;
  return 0;
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

const class_t *voice_c = &_voice_c;
