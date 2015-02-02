/* Implementation of node class interface.
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
#include <string.h>
#include "node.h"
#include "private-node.h"
#include "defs.h"
#include "malloc.h"
#include "class.h"
#include "list.h"
#include "mod.h"

/* Voice - state mapping struct.  */
struct voice_state_s {
  voice_t *voice;
  ptr_t data;
};

/* Mod connection struct.  */
struct modconn_s {
  mod_t *mod;
  uint_t slot;
  ptr_t args;
};

/* Node constructor.  */
static ptr_t
node_constructor (ptr_t ptr, va_list *args)
{
  node_t *self = (node_t *) ptr;
  self->vstates = fz_new_simple_vector (struct voice_state_s);
  self->mods = fz_new_simple_vector (struct modconn_s);
  self->render = NULL;
  self->freestate = NULL;
  return self;
}

/* Node destructor.  */
static ptr_t
node_destructor (ptr_t ptr)
{
  node_t *self = (node_t *) ptr;
  struct voice_state_s *state;
  size_t nvstates = fz_len (self->vstates);
  size_t nmods = fz_len (self->mods);
  uint_t i;

  for (i = 0; i < nvstates; ++i)
    {
      state = fz_ref_at (self->vstates, i, struct voice_state_s);
      if (self->freestate != NULL)
        self->freestate (self, state->data);
      fz_free (state->data);
    }

  /* Modulators are retained in `fz_node_connect'.  */
  for (i = 0; i < nmods; ++i)
    fz_del (fz_ref_at (self->mods, i, struct modconn_s)->mod);

  fz_del (self->vstates);
  fz_del (self->mods);
  return self;
}

/* Get state data for the given NODE and VOICE. If no data
   exists, SIZE bytes are allocetd and returned.  */
ptr_t
fz_node_state_data (node_t *node, voice_t *voice, size_t size)
{
  int_t i;
  size_t nstates;
  struct voice_state_s *state;
  struct voice_state_s newstate;

  if (node == NULL || voice == NULL)
    return NULL;

  nstates = fz_len (node->vstates);
  for (i = 0; i < nstates; ++i)
    {
      state = fz_ref_at (node->vstates, i, struct voice_state_s);
      if (state->voice == voice)
        return state->data;
    }

  if (size == 0)
    return NULL;

  newstate.voice = voice;
  newstate.data = fz_malloc (size);

  i = fz_push_one (node->vstates, &newstate);
  if (i >= 0)
    return fz_ref_at (node->vstates, i,
                      struct voice_state_s)->data;

  return NULL;
}

/* Get `modconn_s' pointer for given SLOT.  */
static struct modconn_s *
get_modconn (node_t *self, uint_t slot)
{
  struct modconn_s *conn;
  size_t nmods;
  uint_t i;

  if (self == NULL)
    return NULL;

  nmods = fz_len (self->mods);
  for (i = 0; i < nmods; ++i)
    {
      conn = fz_ref_at (self->mods, i, struct modconn_s);
      if (conn->slot == slot)
        return conn;
    }

  return NULL;
}

/* Return modulator arg pointer for given SLOT.  */
ptr_t
fz_node_modargs (node_t *self, uint_t slot)
{
  struct modconn_s *conn = get_modconn (self, slot);
  return conn == NULL ? NULL : conn->args;
}

/* Get modulation buffer from SLOT based on SEED.  */
const real_t *
fz_node_modulate (node_t *self, uint_t slot,
                  real_t seed, real_t lo, real_t up)
{
  const list_t *modulation;
  struct modconn_s *conn = get_modconn (self, slot);
  if (conn == NULL)
    return NULL;

  modulation = fz_modulate (conn->mod, seed, lo, up);
  if (modulation == NULL)
    return NULL;

  return (const real_t *) fz_list_data (modulation);
}

/* Connect MOD to SELF on SLOT.  */
int_t
fz_node_connect (node_t *self, mod_t *mod, uint_t slot, ptr_t args)
{
  struct modconn_s *oldconn;
  struct modconn_s newconn = {mod, slot, args};

  if (self == NULL || newconn.mod == NULL)
    return EINVAL;

  oldconn = get_modconn (self, newconn.slot);
  if (oldconn != NULL)
    return EINVAL; /* SLOT already in use.  */

  fz_retain (newconn.mod);
  fz_push_one (self->mods, &newconn);

  return 0;
}

/* Prepare NODE to render NFRAMES frames.  */
void
fz_node_prepare (node_t *node, size_t nframes)
{
  if (node == NULL)
    return;

  uint_t i;
  size_t nmods = fz_len (node->mods);
  for (i = 0; i < nmods; ++i)
    fz_mod_prepare (fz_ref_at (node->mods, i, struct modconn_s)->mod,
                    nframes);
}

/* Render frames from NODE into FRAMES.  */
int_t
fz_node_render (node_t *node,
                list_t *frames,
                const request_t *request)
{
  if (node == NULL || frames == NULL || request == NULL
      || !fz_instance_of (frames, vector_c))
    return -EINVAL;

  size_t nframes = fz_len (frames);
  if (node->render == NULL || nframes == 0)
    return nframes;

  /* Prepare node in case it hasn't been done already.  */
  fz_node_prepare (node, nframes);

  /* Render modulators first.  */
  uint_t i;
  size_t nmods = fz_len (node->mods);
  for (i = 0; i < nmods; ++i)
    fz_mod_render (fz_ref_at (node->mods, i,
                              struct modconn_s)->mod, request);

  return node->render (node, frames, request);
}

/* `node_c' class descriptor.  */
static const class_t _node_c = {
  sizeof (node_t),
  node_constructor,
  node_destructor,
  NULL,
  NULL,
  NULL
};

const class_t *node_c = &_node_c;
