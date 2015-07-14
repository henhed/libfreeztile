/* Implementation of node class interface.
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

#include <errno.h>
#include <string.h>
#include "node.h"
#include "private-node.h"
#include "defs.h"
#include "malloc.h"
#include "class.h"
#include "list.h"
#include "map.h"
#include "mod.h"

/* Mod connection struct.  */
typedef struct {
  mod_t *mod;
  ptr_t args;
} modconn_t;

/* Voice state map init callback.  */
static void
state_init (map_t *map, voice_t *voice, ptr_t state)
{
  node_t *node = fz_map_owner (map);
  if (node->state_init)
    node->state_init (node, voice, state);
  fz_retain (voice);
}

/* Voice state map free callback.  */
static void
state_free (map_t *map, voice_t *voice, ptr_t state)
{
  node_t *node = fz_map_owner (map);
  if (node->state_free)
    node->state_free (node, voice, state);
  fz_del (voice);
}

/* Node constructor.  */
static ptr_t
node_constructor (ptr_t ptr, va_list *args)
{
  node_t *self = (node_t *) ptr;
  self->mods = fz_new (map_c, self, NULL, NULL);
  self->states = fz_new (map_c, self, state_init, state_free);
  self->state_size = 0;
  self->state_init = NULL;
  self->state_free = NULL;
  self->render = NULL;
  return self;
}

/* Node destructor.  */
static ptr_t
node_destructor (ptr_t ptr)
{
  node_t *self = (node_t *) ptr;
  modconn_t *conn;

  /* Modulators are retained in `fz_node_connect'.  */
  fz_map_each (self->mods, conn)
    fz_del (conn->mod);

  fz_del (self->states);
  fz_del (self->mods);
  return self;
}

/* Get state data for the given NODE and VOICE.  */
ptr_t
fz_node_state (node_t *node, const voice_t *voice)
{
  if (!node || !voice || node->state_size == 0)
    return NULL;

  uintptr_t key = (uintptr_t) voice;
  ptr_t state = fz_map_get (node->states, key);
  if (!state)
    state = fz_map_set (node->states, key, NULL, node->state_size);

  return state;
}

/* Return modulator arg pointer for given SLOT.  */
ptr_t
fz_node_modargs (const node_t *self, uint_t slot)
{
  if (!self)
    return NULL;
  modconn_t *conn = fz_map_get (self->mods, slot);
  return conn == NULL ? NULL : conn->args;
}

/* Get modulation buffer from SLOT based on SEED.  */
const real_t *
fz_node_modulate (node_t *self, uint_t slot,
                  real_t seed, real_t lo, real_t up)
{
  if (!self)
    return NULL;

  modconn_t *conn = fz_map_get (self->mods, slot);
  if (!conn)
    return NULL;

  const list_t *modulation = fz_modulate (conn->mod, seed, lo, up);
  if (!modulation)
    return NULL;

  return (const real_t *) fz_list_data (modulation);
}

/* Connect MOD to NODE on SLOT.  */
int_t
fz_node_connect (node_t *node, mod_t *mod, uint_t slot, ptr_t args)
{
  if (!node || !mod || fz_map_get (node->mods, slot))
    return EINVAL;

  modconn_t *conn = fz_map_set (node->mods, slot, NULL,
                                sizeof (modconn_t));
  conn->mod = mod;
  conn->args = args;
  fz_retain (conn->mod);

  return 0;
}

/* Add mods connected to NODE into MODS.  */
int_t
fz_node_collect_mods (const node_t *node, list_t *mods)
{
  if (!node || !mods)
    return EINVAL;

  modconn_t *conn;
  fz_map_each (node->mods, conn)
    if (fz_index_of (mods, conn->mod, fz_cmp_ptr) < 0)
      fz_push_one (mods, conn->mod);

  return 0;
}

/* Prepare NODE to render NFRAMES frames.  */
void
fz_node_prepare (node_t *node, size_t nframes)
{
  if (!node)
    return;
  modconn_t *conn;
  fz_map_each (node->mods, conn)
    fz_mod_prepare (conn->mod, nframes);
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
  modconn_t *conn;
  fz_map_each (node->mods, conn)
    fz_mod_render (conn->mod, request);

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
