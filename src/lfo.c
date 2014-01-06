/* LFO modulator interface implementation.
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
#include <errno.h>
#include "lfo.h"
#include "mod.h"
#include "private-mod.h"
#include "voice.h"
#include "form.h"
#include "node.h"
#include "list.h"

/* LFO class struct.  */
typedef struct lfo_s
{
  mod_t __parent;
  form_t *form;
  real_t freq;
} lfo_t;

/* `fz_mod_render' callback.  */
static int_t
lfo_render (mod_t *mod, const request_t *request)
{
  lfo_t *self = (lfo_t *) mod;
  voice_t **voiceref = fz_mod_state (mod, request->voice, voice_t *);
  request_t formrequest = {};
  real_t *steps;
  int_t nrendered;
  uint_t i;

  if (voiceref == NULL)
    /* Probably because `request->voice' is NULL.  */
    return -EINVAL;
  if (*voiceref == NULL)
    *voiceref = fz_new (voice_c);
  else
    fz_voice_release (*voiceref);

  fz_voice_press (*voiceref, self->freq, 1);

  /* Pass on whatever the request might contain but replace the voice
     with the internal state voice to get the desired frequency.  */
  memcpy (&formrequest, request, sizeof (request_t));
  formrequest.voice = *voiceref;
  nrendered = fz_node_render ((node_t *) self->form,
                             mod->stepbuf,
                             &formrequest);

  /* Node range is -1 - 1 while modulator range should be 0 - 1 so we
     need to convert the form output.  */
  steps = (real_t *) fz_list_data (mod->stepbuf);
  for (i = 0; i < nrendered; ++i)
    steps[i] = (steps[i] / 2) + .5;

  return nrendered;
}

/* Mod destructor callback for cleaning up created voices.  */
static void
lfo_freestate (mod_t *mod, ptr_t voiceref)
{
  lfo_t *lfo = (lfo_t *) mod;
  voice_t *voice = *((voice_t **) voiceref);
  if (voice != NULL)
    fz_del (voice);
}

/* LFO constructor.  */
static ptr_t
lfo_constructor (ptr_t ptr, va_list *args)
{
  lfo_t *self = (lfo_t *)
    ((const class_t *) mod_c)->construct (ptr, args);
  int_t shape = va_arg (*args, int_t);
  real_t freq = va_arg (*args, real_t);

  self->__parent.render = lfo_render;
  self->__parent.freestate = lfo_freestate;
  self->form = fz_new (form_c, shape);
  self->freq = freq;

  return self;
}

/* LFO destructor.  */
static ptr_t
lfo_destructor (ptr_t ptr)
{
  lfo_t *self = (lfo_t *)
    ((const class_t *) mod_c)->destruct (ptr);
  fz_del (self->form);
  return self;
}

/* LFO class descriptor.  */
static const class_t _lfo_c = {
  sizeof (lfo_t),
  lfo_constructor,
  lfo_destructor,
  NULL,
  NULL,
  NULL
};

const class_t *lfo_c = &_lfo_c;
