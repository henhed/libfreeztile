/* Delay node implementation.
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

#include "delay.h"
#include "node.h"
#include "private-node.h"

/* Delay class struct.  */
typedef struct delay_s
{
  node_t __parent;
  real_t feedback;
  real_t gain;
  real_t delay;
} delay_t;

/* Voice state struct for storing delayed samples.  */
struct state_s
{
  list_t *ringbuf;
  uint_t bufpos;
};

/* Aquire state instance for given VOICE and LENGTH.  */
static struct state_s *
get_voice_state (node_t *node, voice_t *voice, size_t length)
{
  size_t curlen;
  uint_t diff;
  struct state_s *state = fz_node_state (node, voice);
  if (!state)
    return NULL;

  if (!state->ringbuf)
    {
      /* Return new empty state.  */
      state->ringbuf = fz_new_simple_vector (real_t);
      fz_clear (state->ringbuf, length);
      state->bufpos = 0;
      return state;
    }

  curlen = fz_len (state->ringbuf);
  if (length > curlen)
    {
      /* Grow delay buffer.  */
      diff = length - curlen;
      fz_insert (state->ringbuf, state->bufpos, diff, NULL);
      state->bufpos = (state->bufpos + diff) % length;
    }
  else if (length < curlen)
    {
      /* Shrink delay buffer.  */
      diff = curlen - length;
      if (diff > state->bufpos)
        {
          diff -= state->bufpos;
          fz_erase (state->ringbuf, curlen - diff, diff);
          diff = state->bufpos;
        }
      state->bufpos -= diff;
      fz_erase (state->ringbuf, state->bufpos, diff);
    }

  return state;
}

/* Delay node renderer.  */
static int_t
delay_render (node_t *node, list_t *frames, const request_t *request)
{
  delay_t *delay = (delay_t *) node;
  struct state_s *state;
  real_t *framedata = fz_list_data (frames);
  size_t nframes = fz_len (frames);
  real_t *buffer;
  size_t buflen;
  uint_t i;
  real_t in;

  if (nframes == 0 || request->srate <= 0)
    return 0;

  buflen = (size_t) (request->srate * delay->delay);
  if (buflen == 0)
    return nframes;

  state = get_voice_state (node, request->voice, buflen);
  if (state == NULL)
    return 0;

  buffer = fz_list_data (state->ringbuf);
  for (i = 0; i < nframes; ++i)
    {
      in = framedata[i];
      framedata[i] += delay->gain * buffer[state->bufpos];
      buffer[state->bufpos] = in + (delay->feedback
                                    * buffer[state->bufpos]);
      state->bufpos = (state->bufpos + 1) % buflen;
    }

  return nframes;
}

/* State cleanup callback.  */
static void
delay_freestate (node_t *node, voice_t *voice, ptr_t state)
{
  (void) node;
  (void) voice;
  fz_del (((struct state_s *) state)->ringbuf);
}

/* Delay constructor. */
static ptr_t
delay_constructor (ptr_t ptr, va_list *args)
{
  delay_t *self = (delay_t *)
    ((const class_t *) node_c)->construct (ptr, args);
  self->__parent.state_size = sizeof (struct state_s);
  self->__parent.state_free = delay_freestate;
  self->__parent.render = delay_render;
  self->feedback = 0;
  self->gain = 0;
  self->delay = 0;
  return self;
}

/* Delay destructor.  */
static ptr_t
delay_destructor (ptr_t ptr)
{
  delay_t *self = (delay_t *)
    ((const class_t *) node_c)->destruct (ptr);
  return self;
}

/* Get DELAYs gain.  */
real_t
fz_delay_get_gain (const delay_t *delay)
{
  return delay ? delay->gain : 0;
}

/* Set DELAYs gain to GAIN.  */
int_t
fz_delay_set_gain (delay_t *delay, real_t gain)
{
  if (!delay || gain < 0)
    return EINVAL;
  delay->gain = gain;
  return 0;
}

/* Get DELAYs feedback.  */
real_t
fz_delay_get_feedback (const delay_t *delay)
{
  return delay ? delay->feedback : 0;
}

/* Set DELAYs feedback to FEEDBACK.  */
int_t
fz_delay_set_feedback (delay_t *delay, real_t feedback)
{
  if (!delay || feedback < 0 || feedback > 1)
    return EINVAL;
  delay->feedback = feedback;
  return 0;
}

/* Get DELAYs delay time.  */
real_t
fz_delay_get_delay (const delay_t *delay)
{
  return delay ? delay->delay : 0;
}

/* Set DELAYs delay time to TIME.  */
int_t
fz_delay_set_delay (delay_t *delay, real_t time)
{
  if (!delay || time < 0)
    return EINVAL;
  delay->delay = time;
  return 0;
}

/* `delay_c' class descriptor.  */
static const class_t _delay_c = {
  sizeof (delay_t),
  delay_constructor,
  delay_destructor,
  NULL,
  NULL,
  NULL
};

const class_t *delay_c = &_delay_c;
