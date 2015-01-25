/* Implementation of graph class interface.
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
#include "graph.h"
#include "list.h"
#include "node.h"

/* Graph class struct.  */
typedef struct graph_s
{
  const class_t *__class;
  list_t *nodes;
  list_t *am; /* Adjacency matrix */
  list_t *sources;
  list_t *sinks;
} graph_t;

/* Graph constructor.  */
static ptr_t
graph_constructor (ptr_t ptr, va_list *args)
{
  graph_t *self = (graph_t *) ptr;
  (void) args;
  self->nodes = fz_new_retaining_vector (node_t *);
  self->am = fz_new_owning_vector (list_t *);
  self->sources = fz_new_pointer_vector (node_t *);
  self->sinks = fz_new_pointer_vector (node_t *);
  return self;
}

/* Graph destructor.  */
static ptr_t
graph_destructor (ptr_t ptr)
{
  graph_t *self = (graph_t *) ptr;
  fz_del (self->sinks);
  fz_del (self->sources);
  fz_del (self->am);
  fz_del (self->nodes);
  return self;
}

/* Check if NODE is part of GRAPH.  */
bool_t
fz_graph_has_node (const graph_t *graph, const node_t *node)
{
  if (graph == NULL || node == NULL
      || fz_index_of (graph->nodes, (const ptr_t) node,
                      fz_cmp_ptr) < 0)
    return FALSE;

  return TRUE;
}

/* Add NODE to GRAPH.  */
uint_t
fz_graph_add_node (graph_t *graph, node_t *node)
{
  if (graph == NULL || node == NULL
      || fz_graph_has_node (graph, node))
    return EINVAL;

  int_t index = fz_push_one (graph->nodes, node);
  if (index < 0)
    return -index;

  /* Expand adjacency matrix.  */
  int_t x;
  int_t y;
  int_t w;
  int_t h = fz_len (graph->am);
  list_t *edges;
  real_t edge = 0;

  for (y = 0; y <= index; ++y)
    {
      if (y >= h)
        fz_push_one (graph->am, fz_new_simple_vector (real_t));
      edges = fz_ref_at (graph->am, y, list_t);
      w = fz_len (edges);
      for (x = w; x <= index; ++x)
        {
          fz_push_one (edges, &edge);
        }
    }

  return 0;
}

/* Remove NODE from GRAPH.  */
uint_t
fz_graph_del_node (graph_t *graph, node_t *node)
{
  if (graph == NULL || node == NULL)
    return EINVAL;

  int_t index = fz_index_of (graph->nodes, node, fz_cmp_ptr);
  if (index < 0)
    return EINVAL;

  index = fz_erase_one (graph->nodes, index);
  if (index < 0)
    return -index;

  /* Remove NODEs edges from adjacency matrix.  */
  fz_erase_one (graph->am, index);
  int_t y = 0;
  int_t h = fz_len (graph->am);
  list_t *edges;
  for (; y < h; ++y)
    {
      edges = fz_ref_at (graph->am, y, list_t);
      fz_erase_one (edges, index);
    }

  return 0;
}

/* Graph class descriptor.  */
static const class_t _graph_c = {
  sizeof (graph_t),
  graph_constructor,
  graph_destructor,
  NULL,
  NULL,
  NULL
};

const class_t *graph_c = &_graph_c;
