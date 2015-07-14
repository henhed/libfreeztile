/* Filter node implementation.
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

#include "filter.h"
#include "node.h"
#include "private-node.h"

/* Filter class struct.  */
typedef struct filter_s
{
  node_t __parent;
  int_t type;
  real_t frequency;
  real_t resonance;
} filter_t;

/* Voice state struct for keeping track of old coefficient values.  */
struct state_s
{
  real_t b0;
  real_t b1;
  real_t b2;
  real_t b3;
  real_t b4;
};

static const struct state_s zero_state = {0, 0, 0, 0, 0};

/* Filter node renderer.  */
static int_t
filter_render (node_t *node, list_t *frames, const request_t *request)
{
  filter_t *filter = (filter_t *) node;
  struct state_s *state;
  size_t nframes = fz_len (frames);
  real_t *framedata = fz_list_data (frames);
  if (nframes == 0 || request->srate <= 0)
    return 0;

  uint_t i = 0;
  real_t freq = filter->frequency / request->srate;
  real_t q = 1. - freq;
  real_t p = freq + (.8 * freq * q);
  real_t f = p + p - 1.;
  q = filter->resonance * (1. + (.5 * q * (1. - q + 5.6 * q * q)));

  state = fz_node_state (node, request->voice);
  if (state == NULL)
    return 0;

  real_t in,
    t1,
    t2,
    b0 = state->b0,
    b1 = state->b1,
    b2 = state->b2,
    b3 = state->b3,
    b4 = state->b4;
  if (memcmp (state, &zero_state, sizeof (struct state_s)) == 0)
    {
      /* First call for given voice, initialize coefficients.  */
      i = 1;
      b0 = framedata[0];
      b1 = b2 = b3 = b4 = 0;
    }

  for (; i < nframes; ++i)
    {
      in = framedata[i];
      in -= q * b4; /* Feedback */
      t1 = b1;
      b1 = (in + b0) * p - b1 * f;
      t2 = b2;
      b2 = (b1 + t1) * p - b2 * f;
      t1 = b3;
      b3 = (b2 + t2) * p - b3 * f;
      b4 = (b3 + t1) * p - b4 * f;
      b4 = b4 - b4 * b4 * b4 * 0.166667; /* Clipping */
      b0 = in;
      switch (filter->type)
        {
        case FILTER_TYPE_HIGHPASS:
          framedata[i] = in - b4;
          break;
        case FILTER_TYPE_BANDPASS:
          framedata[i] = 3.0 * (b3 - b4);
          break;
        default: /* FILTER_TYPE_LOWPASS */
          framedata[i] = b4;
          break;
        }
    }

  /* Store coefficients in voice state.  */
  state->b0 = b0;
  state->b1 = b1;
  state->b2 = b2;
  state->b3 = b3;
  state->b4 = b4;

  return nframes;
}

/* Filter constructor. */
static ptr_t
filter_constructor (ptr_t ptr, va_list *args)
{
  filter_t *self = (filter_t *)
    ((const class_t *) node_c)->construct (ptr, args);
  self->__parent.state_size = sizeof (struct state_s);
  self->__parent.render = filter_render;
  self->type = FILTER_TYPE_LOWPASS;
  self->frequency = 20000.0;
  self->resonance = .0;
  return self;
}

/* Filter destructor.  */
static ptr_t
filter_destructor (ptr_t ptr)
{
  filter_t *self = (filter_t *)
    ((const class_t *) node_c)->destruct (ptr);
  return self;
}

/* Get FILTERs type.  */
int_t
fz_filter_get_type (const filter_t *filter)
{
  return filter ? filter->type : -1;
}

/* Set FILTERs type.  */
int_t
fz_filter_set_type (filter_t *filter, int_t type)
{
  if (!filter
      || (type != FILTER_TYPE_LOWPASS
          && type != FILTER_TYPE_HIGHPASS
          && type != FILTER_TYPE_BANDPASS))
    return EINVAL;
  filter->type = type;
  return 0;
}

/* Get FILTERs cutoff frequency.  */
real_t
fz_filter_get_frequency (const filter_t *filter)
{
  return filter ? filter->frequency : 0;
}

/* Set FILTERs cutoff frequency.  */
int_t
fz_filter_set_frequency (filter_t *filter, real_t frequency)
{
  if (!filter || frequency <= 0)
    return EINVAL;
  filter->frequency = frequency;
  return 0;
}

/* Get FILTERs resonance.  */
real_t
fz_filter_get_resonance (const filter_t *filter)
{
  return filter ? filter->resonance : 0;
}

/* Set FILTERs resonance.  */
int_t
fz_filter_set_resonance (filter_t *filter, real_t resonance)
{
  if (!filter)
    return EINVAL;
  filter->resonance = resonance;
  return 0;
}

/* `filter_c' class descriptor.  */
static const class_t _filter_c = {
  sizeof (filter_t),
  filter_constructor,
  filter_destructor,
  NULL,
  NULL,
  NULL
};

const class_t *filter_c = &_filter_c;
