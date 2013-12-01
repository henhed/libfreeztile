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
#include "node.h"
#include "defs.h"
#include "class.h"
#include "list.h"

/* node class struct.  */
struct node_s
{
  const class_t *__class;
  list_t *parents;
  list_t *children;
};

/* Node constructor.  */
static ptr_t
node_constructor (ptr_t ptr, va_list *args)
{
  node_t *self = (node_t *) ptr;
  self->parents = fz_new_vector (ptr_t);
  self->children = fz_new_vector (ptr_t);
  return self;
}

/* Node destructor.  */
static ptr_t
node_destructor (ptr_t ptr)
{
  node_t *self = (node_t *) ptr;
  fz_del (self->parents);
  fz_del (self->children);
  return self;
}

/* Return TRUE if NODE is an ancestor of DESCENDANT.  */
static bool_t
is_ancestor_of (node_t *node, node_t *descendant)
{
  uint_t i;
  size_t len;
  node_t *child;

  if (node == NULL || descendant == NULL || node == descendant)
    return FALSE;

  len = fz_len (node->children);
  if (len == 0)
    return FALSE;

  for (i = 0; i < len; ++i) {
    child = fz_at_val (node->children, i, node_t *);
    if (child == descendant || is_ancestor_of (child, descendant))
      return TRUE;
  }

  return FALSE;
}

/* Return TRUE if NODE is a descendant of ANCESTOR.  */
static bool_t
is_descendant_of (node_t *node, node_t *ancestor)
{
  return is_ancestor_of (ancestor, node);
}

/* Append CHILD to NODE.  */
int_t
fz_node_fork (node_t *node, node_t *child)
{
  int_t err;

  /* CHILD can not be appended to NODE if (1) CHILD and NODE refer to
     the same object, (2) CHILD already has a parent or (3) CHILD is
     an ancestor of NODE.  */
  if (node == NULL || child == NULL || node == child)
    return -EINVAL;

  if (fz_len (child->parents) > 0 || is_ancestor_of (child, node))
    return -EINVAL;

  err = fz_push_one (child->parents, &node);
  if (err < 0)
    return err;

  return fz_push_one (node->children, &child);
}

/* Connect PARENT to NODE.  */
int_t
fz_node_join (node_t *node, node_t *parent)
{
  return -ENOSYS;
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
