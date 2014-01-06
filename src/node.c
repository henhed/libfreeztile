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

#define NODE_NONE 0
#define NODE_RENDERED (1 << 0)

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
  self->parents = fz_new_pointer_vector (node_t *);
  self->children = fz_new_owning_vector (node_t *);
  self->framebuf = fz_new_simple_vector (real_t);
  self->vstates = fz_new_simple_vector (struct voice_state_s);
  self->mods = fz_new_simple_vector (struct modconn_s);
  self->flags = NODE_NONE;
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

  fz_del (self->parents);
  fz_del (self->children);
  fz_del (self->framebuf);
  fz_del (self->vstates);
  fz_del (self->mods);
  return self;
}

/* Return TRUE if NODE is an ancestor of DESCENDANT.  */
static bool_t
is_ancestor_of (const node_t *node, const node_t *descendant)
{
  uint_t i;
  size_t len;
  node_t *child;

  if (node == NULL || descendant == NULL || node == descendant)
    return FALSE;

  len = fz_len (node->children);
  if (len == 0)
    return FALSE;

  for (i = 0; i < len; ++i)
    {
      child = fz_ref_at (node->children, i, node_t);
      if (child == descendant || is_ancestor_of (child, descendant))
        return TRUE;
    }

  return FALSE;
}

/* Return TRUE if NODE is a descendant of ANCESTOR.  */
static bool_t
is_descendant_of (const node_t *node, const node_t *ancestor)
{
  return is_ancestor_of (ancestor, node);
}

/* Get a list of all root nodes of NODE.  */
static list_t *
get_root_nodes (const node_t *node, list_t *roots)
{
  uint_t i;
  size_t num_parents;
  node_t *parent;

  if (roots == NULL)
    roots = fz_new_pointer_vector (node_t *);

  if (node != NULL)
    {
      num_parents = fz_len (node->parents);
      for (i = 0; i < num_parents; ++i)
        {
          parent = fz_ref_at (node->parents, i, node_t);
          if (fz_len (parent->parents) == 0)
            fz_push_one (roots, parent);
          else
            roots = get_root_nodes (parent, roots);
        }
  }

  return roots;
}

/* Get a list of all nodes of NODE.  */
static list_t *
get_leaf_nodes (const node_t *node, list_t *leaves)
{
  uint_t i;
  size_t num_children;
  node_t *child;

  if (leaves == NULL)
    leaves = fz_new_pointer_vector (node_t *);

  if (node != NULL)
    {
      num_children = fz_len (node->children);
      for (i = 0; i < num_children; ++i)
        {
          child = fz_ref_at (node->children, i, node_t);
          if (fz_len (child->children) == 0
              && fz_index_of (leaves, child, fz_cmp_ptr) < 0)
            fz_push_one (leaves, child);
          else
            leaves = get_leaf_nodes (child, leaves);
        }
    }

  return leaves;
}

/* Return TRUE if CHILD can fork NODE.  */
bool_t
fz_node_can_fork (const node_t *node, const node_t *child)
{
  /* CHILD can not be appended to NODE if (1) CHILD and NODE refer to
     the same object, (2) CHILD already has a parent or (3) CHILD is
     an ancestor of NODE.  */
  if (node == NULL || child == NULL || node == child)
    return FALSE;

  if (fz_len (child->parents) > 0 || is_ancestor_of (child, node))
    return FALSE;

  return TRUE;
}

/* Append CHILD to NODE.  */
int_t
fz_node_fork (node_t *node, node_t *child)
{
  int_t err;

  if (fz_node_can_fork (node, child) == FALSE)
    return -EINVAL;

  err = fz_push_one (child->parents, node);
  if (err < 0)
    return err;

  return fz_push_one (node->children, child);
}

/* Return TRUE if PARENT can be joined into NODE.  */
bool_t
fz_node_can_join (const node_t *node, const node_t *parent)
{
  uint_t i;
  size_t num_roots;
  list_t *node_roots;
  node_t *root;
  bool_t can_join = FALSE;

  /* PARENT can be joined into NODE if (1) NODE is already a child
     (fork) of some node other than PARENT, (2) NODE and PARENT have
     at least one common ancestor (PARENT can be the common acestor)
     and (3) PARENT is not a descendant of NODE.  */

  if (node == NULL || parent == NULL || node == parent)
    return can_join;

  if (fz_len (node->parents) == 0
      || fz_index_of (node->parents,
                      (const ptr_t) parent,
                      fz_cmp_ptr) >= 0
      || is_ancestor_of (node, parent))
      return can_join;

  node_roots = get_root_nodes (node, NULL);
  if (fz_index_of (node_roots, (const ptr_t) parent, fz_cmp_ptr) >= 0)
    /* PARENT is one of NODEs roots.  */
    can_join = TRUE;
  else
    {
      /* Check if PARENT is a descendant of any of NODEs roots.  */
      num_roots = fz_len (node_roots);
      for (i == 0; i < num_roots; ++i)
        {
          root = fz_ref_at (node_roots, i, node_t);
          if (is_ancestor_of (root, parent))
            {
              can_join = TRUE;
              break;
            }
        }
    }

  fz_del (node_roots);

  return can_join;
}

/* Connect PARENT to NODE.  */
int_t
fz_node_join (node_t *node, node_t *parent)
{
  int_t err;

  if (fz_node_can_join (node, parent) == FALSE)
    return -EINVAL;

  err = fz_push_one (node->parents, parent);
  if (err < 0)
    return err;

  return fz_push_one (parent->children, node);
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

/* Reset NODE and all its ancestors as preparation for `render_node'.  */
static void
reset_node (node_t *node, size_t nframes)
{
  uint_t i = 0;
  size_t nparents = fz_len (node->parents);

  for (; i < nparents; ++i)
    reset_node (fz_ref_at (node->parents, i, node_t), nframes);

  node->flags &= ~NODE_RENDERED;
  fz_clear (node->framebuf, nframes);
}

/* Render NODE and all its ancestors into NODEs internal buffer using
   FRAMES as source for root nodes.  */
static int_t
render_node (node_t *node,
             const list_t *frames,
             const request_t *request)
{
  uint_t i, j;
  node_t *parent;
  size_t nframes = fz_len ((const ptr_t) frames);
  size_t nparents = fz_len (node->parents);
  size_t nmods = fz_len (node->mods);
  real_t *indata, *outdata, *pdata;

  if (node->flags & NODE_RENDERED)
    return (int_t) fz_len (node->framebuf);

  outdata = fz_list_data (node->framebuf);
  if (outdata == NULL)
    return -EINVAL;

  if (nparents == 0)
    {
      indata = fz_list_data (frames);
      if (indata == NULL)
        return -EINVAL;
      memcpy (outdata, indata, nframes * sizeof (real_t));
    }
  else
    for (i = 0; i < nparents; ++i)
      {
        parent = fz_ref_at (node->parents, i, node_t);
        render_node (parent, frames, request);
        pdata = fz_list_data (parent->framebuf);
        if (pdata == NULL)
          continue;
        for (j = 0; j < nframes; ++j)
          outdata[j] += pdata[j];
      }

  node->flags |= NODE_RENDERED;
  if (node->render != NULL)
    {
      /* Render modulators first.  */
      for (i = 0; i < nmods; ++i)
        /* FIXME: Render mod only once even though it might be connected
           to multiple slots.  */
        fz_mod_render (fz_ref_at (node->mods, i,
                                  struct modconn_s)->mod,
                       nframes,
                       request);

      return node->render (node, request);
    }

  return nframes;
}

/* Render frames from NODE into FRAMES.  */
int_t
fz_node_render (node_t *node,
                list_t *frames,
                const request_t *request)
{
  uint_t i;
  int_t err;
  list_t *leaves;
  size_t nleaves;
  node_t *anchor;
  real_t *fdata, *adata;

  if (node == NULL || frames == NULL || request == NULL)
    return -EINVAL;

  fdata = fz_list_data (frames);
  if (fdata == NULL)
    return -EINVAL; /* FRAMES is empty or not a vector.  */

  /* Find anchor node.  */
  leaves = get_leaf_nodes (node, NULL);
  nleaves = fz_len (leaves);

  if (nleaves == 0)
    anchor = node;
  else if (nleaves == 1)
    anchor = fz_ref_at (leaves, 0, node_t);
  else
    {
      /* TODO: If NLEAVES is greater than 1, inflate frames as
         requested by request->access instead of just joining
         leaves into a new anchor.  */
      anchor = fz_new (node_c);
      fz_node_fork (fz_ref_at (leaves, 0, node_t), anchor);
      for (i = 1; i < nleaves; ++i)
        fz_node_join (anchor, fz_ref_at (leaves, i, node_t));
    }

  fz_del (leaves); /* Allocated in `get_leaf_nodes'.  */

  /* Render node tree.  */
  reset_node (anchor, fz_len (frames));
  err = render_node (anchor, frames, request);
  if (err < 0)
    return err;

  adata = fz_list_data (anchor->framebuf);
  if (adata != NULL)
    /* ADATA can only be NULL if the anchors frame buffer is empty
       but `reset_node' clears it to the size of FRAMES so that should
       never happen.  */
    memcpy (fdata, adata, err * sizeof (real_t));

  return err;
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
