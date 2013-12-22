/* Implementation of node class interface.
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
#include <string.h>
#include "node.h"
#include "private-node.h"
#include "defs.h"
#include "class.h"
#include "list.h"

#define NODE_NONE 0
#define NODE_RENDERED (1 << 0)

/* Node constructor.  */
static ptr_t
node_constructor (ptr_t ptr, va_list *args)
{
  node_t *self = (node_t *) ptr;
  self->parents = fz_new_pointer_vector (node_t *);
  self->children = fz_new_owning_vector (node_t *);
  self->framebuf = fz_new_simple_vector (real_t);
  self->flags = NODE_NONE;
  self->render = NULL;
  return self;
}

/* Node destructor.  */
static ptr_t
node_destructor (ptr_t ptr)
{
  node_t *self = (node_t *) ptr;
  fz_del (self->parents);
  fz_del (self->children);
  fz_del (self->framebuf);
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
          if (fz_len (child->children) == 0)
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

/* Reset NODE and all its ancestors as preparation for `render_node'.  */
static void
reset_node (node_t *node, size_t num_frames)
{
  uint_t i = 0;
  size_t num_parents = fz_len (node->parents);

  for (; i < num_parents; ++i)
    reset_node (fz_ref_at (node->parents, i, node_t), num_frames);

  node->flags &= ~NODE_RENDERED;
  fz_clear (node->framebuf, num_frames);
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
  size_t num_frames = fz_len (frames);
  size_t num_parents = fz_len (node->parents);

  if (node->flags & NODE_RENDERED)
    return (int_t) fz_len (node->framebuf);

  node->flags |= NODE_RENDERED;

  if (num_parents == 0)
    {
      /* FIXME: Use list copy function instead of `memcpy'.  */
      memcpy (fz_at (node->framebuf, 0),
              fz_at (frames, 0),
              num_frames * sizeof (real_t));
    }
  else
    for (i = 0; i < num_parents; ++i)
      {
        parent = fz_ref_at (node->parents, i, node_t);
        render_node (parent, frames, request);
        for (j = 0; j < num_frames; ++j)
          fz_val_at (node->framebuf, j, real_t)
            += fz_val_at (parent->framebuf, j, real_t);
      }

  if (node->render != NULL)
    return node->render(node, request);

  return (int_t) fz_len (node->framebuf);
}

/* Render frames from NODE into BUFFER.  */
int_t
fz_node_render (node_t *node,
                list_t *buffer,
                const request_t *request)
{
  uint_t i;
  int_t err;
  size_t nleaves;
  node_t *anchor;
  list_t *leaf_nodes;

  if (node == NULL || buffer == NULL || request == NULL)
    return -EINVAL;

  /* Find anchor node.  */
  leaf_nodes = get_leaf_nodes (node, NULL);
  nleaves = fz_len (leaf_nodes);

  if (nleaves == 0)
    anchor = node;
  else if (nleaves == 1)
    anchor = fz_ref_at (leaf_nodes, 0, node_t);
  else
    {
      anchor = fz_new (node_c);
      fz_node_fork (fz_ref_at (leaf_nodes, 0, node_t), anchor);
      for (i = 1; i < nleaves; ++i)
        fz_node_join (anchor, fz_ref_at (leaf_nodes, i, node_t));
    }

  fz_del (leaf_nodes);

  /* Render node tree.  */
  reset_node (anchor, fz_len (buffer));
  err = render_node (anchor, buffer, request);
  if (err < 0)
    return err;

  /* FIXME: Use list copy function instead of `memcpy'.  */
  memcpy (fz_at (buffer, 0),
          fz_at (anchor->framebuf, 0),
          err * sizeof (real_t));

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
