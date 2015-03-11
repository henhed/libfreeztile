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

#define VOICE_FLAG_NONE 0
#define VOICE_FLAG_PRESSED (1 << 0)
#define VOICE_FLAG_KILLED (1 << 1)
#define VOICE_FLAG_REPOSSESSED (1 << 2)

#define VPOOL_STACK_CAPACITY 32

#define FREQ_BY_ID(id) \
  (A4_FREQ * pow (TWELFTH_ROOT_OF_TWO, ((int_t) (id)) - A4_ID))


/* voice class struct.  */
struct voice_s
{
  const class_t *__class;
  uint_t id;
  real_t frequency;
  real_t velocity;
  real_t pressure;
  flags_t flags;
};

/* voice pool class struct.  */
struct vpool_s
{
  const class_t *__class;
  list_t *pool;
  list_t *active_voices;
  uint_t priority;
  list_t *stack;
};

/* Data for stolen voices in vpool stack.  */
typedef struct stack_voice_s
{
  uint_t id;
  real_t pressure;
} stack_voice_t;

/* Voice constructor.  */
static ptr_t
voice_constructor (ptr_t ptr, va_list *args)
{
  voice_t *self = (voice_t *) ptr;
  self->id = 0;
  self->frequency = 440;
  self->velocity = 0;
  self->pressure = self->velocity;
  self->flags = VOICE_FLAG_NONE;
  return self;
}

/* 'Press' VOICE with a given FREQUENCY and VELOCITY.  */
int_t
fz_voice_press (voice_t *voice, real_t frequency, real_t velocity)
{
  if (voice == NULL
      || (voice->flags & VOICE_FLAG_PRESSED)
      || frequency <= 0
      || (velocity <= 0 || velocity > 1))
    return EINVAL;

  voice->flags |= VOICE_FLAG_PRESSED;
  voice->frequency = frequency;
  voice->pressure = voice->velocity = velocity;

  return 0;
}

/* 'Touch' a pressed VOICE with a given amount of PRESSURE.  */
int_t
fz_voice_aftertouch (voice_t *voice, real_t pressure)
{
  if (voice == NULL
      || (~voice->flags & VOICE_FLAG_PRESSED)
      || (pressure <= 0 || pressure > 1))
    return EINVAL;

  voice->pressure = pressure;

  return 0;
}

/* 'Release' a pressed VOICE.  */
int_t
fz_voice_release (voice_t *voice)
{
  if (voice == NULL || (~voice->flags & VOICE_FLAG_PRESSED))
    return EINVAL;

  voice->flags &= ~VOICE_FLAG_PRESSED;
  voice->pressure = voice->velocity = 0;

  return 0;
}

/* Report the state of VOICE.  */
bool_t
fz_voice_pressed (const voice_t *voice)
{
  return voice != NULL && (voice->flags & VOICE_FLAG_PRESSED)
    ? TRUE
    : FALSE;
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

/* Check if VOICE has been repossessed by a previous key.  */
bool_t
fz_voice_repossessed (const voice_t *voice)
{
  return voice && (voice->flags & VOICE_FLAG_REPOSSESSED)
    ? TRUE
    : FALSE;
}

/* Voice pool constructor.  */
static ptr_t
vpool_constructor (ptr_t ptr, va_list *args)
{
  vpool_t *self = (vpool_t *) ptr;
  size_t polyphony = va_arg (*args, size_t);
  self->pool = fz_new_owning_vector (voice_t *);
  self->active_voices = fz_new_owning_vector (voice_t *);
  self->stack = fz_new_simple_vector (stack_voice_t);
  for (; polyphony > 0; --polyphony)
    fz_push_one (self->pool, fz_new (voice_c));
  /* Pre-allocate space for active and stolen voices.  */
  fz_clear (self->active_voices, fz_len (self->pool));
  fz_clear (self->active_voices, 0);
  fz_clear (self->stack, VPOOL_STACK_CAPACITY);
  fz_clear (self->stack, 0);
  self->priority = VOICE_POOL_PRIORITY_FIFO;
  return self;
}

/* Voice pool destructor.  */
static ptr_t
vpool_destructor (ptr_t ptr)
{
  vpool_t *self = (vpool_t *) ptr;
  fz_del (self->stack);
  fz_del (self->active_voices);
  fz_del (self->pool);
  return self;
}

/* Active voice sort callback for pressure priority.  */
static int_t
voice_compare_pressure (const ptr_t a, const ptr_t b)
{
  const voice_t *va = *(const voice_t **) a;
  const voice_t *vb = *(const voice_t **) b;
  if (va->pressure < vb->pressure)
    return -1;
  else if (vb->pressure < va->pressure)
    return 1;
  return 0;
}

/* Prioritize active voices in POOL.  */
static inline void
vpool_prioritize (vpool_t *pool)
{
  /* Move killed voices back to pool.  */
  voice_t *voice;
  int_t i = ((int_t) fz_len (pool->active_voices)) - 1;
  for (; i >= 0; --i)
    {
      voice = fz_ref_at (pool->active_voices, i, voice_t);
      if (voice->flags & VOICE_FLAG_KILLED)
        {
          voice->flags &= ~VOICE_FLAG_KILLED;
          fz_retain (voice);
          fz_erase_one (pool->active_voices, i);
          fz_push_one (pool->pool, voice);
        }
    }

  /* Prioritize remaining active voices.  */
  cmp_f prioritizer;
  switch (pool->priority)
    {
    case VOICE_POOL_PRIORITY_PRESSURE:
      prioritizer = voice_compare_pressure;
      break;
    default: /* VOICE_POOL_PRIORITY_FIFO */
      prioritizer = NULL;
      break;
    }
  if (prioritizer != NULL)
    fz_sort (pool->active_voices, prioritizer);
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

  int_t i;
  size_t poolsize;
  size_t nactive = fz_len (pool->active_voices);
  voice_t *voice = vpool_get_active_voice (pool, id);
  real_t frequency = FREQ_BY_ID (id);
  stack_voice_t stolen;

  if (voice != NULL)
    {
      if (fz_voice_pressed (voice))
        return EINVAL;
      i = fz_index_of (pool->active_voices, voice, fz_cmp_ptr);
      if (nactive > 1 && i < nactive - 1)
        {
          // Move the pressed voice to the bootom of the
          // `active_voices' list for FIFO priority.
          fz_retain (voice);
          fz_erase_one (pool->active_voices, i);
          fz_push_one (pool->active_voices, voice);
        }
      voice->flags &= ~VOICE_FLAG_KILLED;
      return fz_voice_press (voice, frequency, velocity);
    }

  vpool_prioritize (pool);
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
      /* Steal lowest prioritized voice.  */
      voice = fz_ref_at (pool->active_voices, 0, voice_t);
      fz_retain (voice);
      fz_erase_one (pool->active_voices, 0);
      if (fz_voice_pressed (voice))
        {
          /* If the stolen voice is still pressed we'll remember it
             and possibly revive it later.  */
          stolen.id = voice->id;
          stolen.pressure = voice->pressure;
          fz_push_one (pool->stack, &stolen);
          fz_voice_release (voice);
        }
    }

  voice->id = id;
  voice->flags &= ~VOICE_FLAG_REPOSSESSED;
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
  int_t i;
  size_t nstolen = fz_len (pool->stack);
  stack_voice_t *stolen;
  if (voice == NULL)
    {
      for (i = nstolen - 1; i >= 0; --i)
        {
          stolen = fz_ref_at (pool->stack, i, stack_voice_t);
          if (stolen->id == id)
            fz_erase_one (pool->stack, i);
        }
      return 0;
    }

  if (nstolen > 0)
    {
      stolen = fz_ref_at (pool->stack, nstolen - 1, stack_voice_t);
      voice->id = stolen->id;
      voice->pressure = stolen->pressure;
      voice->frequency = FREQ_BY_ID (voice->id);
      voice->flags |= VOICE_FLAG_REPOSSESSED;
      fz_erase_one (pool->stack, nstolen - 1);
      return 0;
    }

  return fz_voice_release (voice);
}

/* Inactivate VOICE in POOL.  */
int_t
fz_vpool_kill (vpool_t *pool, voice_t *voice)
{
  if (!pool || !voice)
    return EINVAL;

  int_t index = fz_index_of (pool->active_voices, (ptr_t) voice,
                             fz_cmp_ptr);
  if (index >= 0)
    {
      fz_voice_release (voice);
      voice->flags |= VOICE_FLAG_KILLED;
    }

  return 0;
}

/* Inactivate voice with ID in POOL.  */
int_t
fz_vpool_kill_id (vpool_t *pool, uint_t id)
{
  if (!pool)
    return EINVAL;

  voice_t * voice = vpool_get_active_voice (pool, id);
  if (voice)
    return fz_vpool_kill (pool, voice);

  return 0;
}

/* Get active voices from POOL.  */
const list_t *
fz_vpool_voices (vpool_t *pool)
{
  if (!pool)
    return NULL;
  vpool_prioritize (pool);
  return pool->active_voices;
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
