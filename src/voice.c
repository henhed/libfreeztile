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

#include <string.h>
#include <math.h>
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

const class_t *voice_c = &_voice_c;
