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

#define GRAPH_NODE_NONE 0
#define GRAPH_NODE_RENDERED (1 << 0)

/* Graph class struct.  */
typedef struct graph_s
{
  const class_t *__class;
  list_t *nodes;
  list_t *am; /* Adjacency matrix */
  list_t *buffers;
  list_t *flags;
} graph_t;

/* Graph constructor.  */
static ptr_t
graph_constructor (ptr_t ptr, va_list *args)
{
  graph_t *self = (graph_t *) ptr;
  (void) args;
  self->nodes = fz_new_retaining_vector (node_t *);
  self->am = fz_new_owning_vector (list_t *);
  self->buffers = fz_new_owning_vector (list_t *);
  self->flags = fz_new_simple_vector (flags_t);
  return self;
}

/* Graph destructor.  */
static ptr_t
graph_destructor (ptr_t ptr)
{
  graph_t *self = (graph_t *) ptr;
  fz_del (self->flags);
  fz_del (self->buffers);
  fz_del (self->am);
  fz_del (self->nodes);
  return self;
}

/* Get NODEs index in GRAPH.  */
static int_t
graph_node_index (const graph_t *graph, const node_t *node)
{
  if (graph == NULL || node == NULL)
    return -EINVAL;
  return fz_index_of (graph->nodes, (const ptr_t) node, fz_cmp_ptr);
}

/* Get NODEs outgoing edges in GRAPH.  */
static const list_t *
graph_node_edges (const graph_t *graph, const node_t *node)
{
  int_t index = graph_node_index (graph, node);
  return index >= 0 && (uint_t) index < fz_len (graph->am)
    ? fz_ref_at (graph->am, index, list_t)
    : NULL;
}

/* Check if NODE is a sink in GRAPH.  */
static bool_t
graph_node_is_sink (const graph_t *graph, const node_t *node)
{
  const list_t *edges = graph_node_edges (graph, node);
  if (edges == NULL)
    return FALSE; /* NODE is not part of GRAPH.  */

  uint_t index;
  size_t nedges = fz_len ((const ptr_t) edges);
  for (index = 0; index < nedges; ++index)
    if (fz_val_at (edges, index, real_t) >= 0)
      return FALSE; /* NODE has at least 1 outgoing edge.  */

  return TRUE;
}

/* Get pointer to edge connecting SOURCE with SINK in GRAPH.  */
static real_t *
graph_edge_ptr (const graph_t *graph, const node_t *source,
                const node_t *sink)
{
  const list_t *edges = graph_node_edges (graph, source);
  if (edges == NULL)
    return NULL;

  int_t index = graph_node_index (graph, sink);
  if (index < 0 || (uint_t) index >= fz_len ((const ptr_t) edges))
    return NULL;

  return fz_ref_at (edges, index, real_t);
}

/* Check if SINK can be reached from SOURCE in GRAPH.  */
static bool_t
graph_path_exists (const graph_t *graph, const node_t *source,
                   const node_t *sink)
{
  if (graph == NULL || source == NULL || sink == NULL)
    return FALSE;

  int_t srcidx = graph_node_index (graph, source);
  int_t snkidx = graph_node_index (graph, sink);
  if (srcidx < 0 || snkidx < 0)
    return FALSE;
  else if (srcidx == snkidx)
    return TRUE;

  const list_t *edges = graph_node_edges (graph, source);
  if (fz_val_at (edges, snkidx, real_t) >= 0)
    return TRUE;

  uint_t index;
  size_t nedges = fz_len ((const ptr_t) edges);
  node_t *adjacent;
  for (index = 0; index < nedges; ++index)
    {
      if (index == (uint_t) srcidx || index == (uint_t) snkidx
          || fz_val_at (edges, index, real_t) < 0)
        /* We've already checked for self-loop, adjacency and edge < 0
           means there's no connection.  */
        continue;
      adjacent = fz_ref_at (graph->nodes, index, node_t);
      if (graph_path_exists (graph, adjacent, sink))
        return TRUE;
    }

  return FALSE;
}

/* Check if NODE is part of GRAPH.  */
bool_t
fz_graph_has_node (const graph_t *graph, const node_t *node)
{
  if (graph == NULL || node == NULL
      || graph_node_index (graph, node) < 0)
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
  real_t edge = -1; /* A negative value indicates no connection.  */

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

  /* Add a frame buffer and flags for NODE.  */
  flags_t noflags = GRAPH_NODE_NONE;
  fz_push_one (graph->buffers, fz_new_simple_vector (real_t));
  fz_push_one (graph->flags, &noflags);

  return 0;
}

/* Remove NODE from GRAPH.  */
uint_t
fz_graph_del_node (graph_t *graph, node_t *node)
{
  if (graph == NULL || node == NULL)
    return EINVAL;

  int_t index = graph_node_index (graph, node);
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

  /* Remove NODEs frame buffer and flags.  */
  fz_erase_one (graph->buffers, index);
  fz_erase_one (graph->flags, index);

  return 0;
}

/* Check if SINK can be connected to SOURCE in GRAPH.  */
bool_t
fz_graph_can_connect (const graph_t *graph, const node_t *source,
                      const node_t *sink)
{
  if (source == sink || !fz_graph_has_node (graph, source)
      || !fz_graph_has_node (graph, sink))
    return FALSE;
  /* If SOURCE can be found downstream from SINK, connecting the two
     would create a loop.  */
  return graph_path_exists (graph, sink, source) ? FALSE : TRUE;
}

/* Connect SINK to SOURCE in GRAPH.  */
uint_t
fz_graph_connect (graph_t *graph, const node_t *source,
                  const node_t *sink)
{
  if (!fz_graph_can_connect (graph, source, sink))
    return EINVAL;

  real_t *edge = graph_edge_ptr (graph, source, sink);
  if (edge == NULL)
    return 1; /* Inconsistent adjacency matrix.  */

  /* Set initial weight to 1 (100% mix).  */
  *edge = 1.0;

  return 0;
}

/* Get NODEs frame buffer from GRAPH.  */
const list_t *
fz_graph_buffer (const graph_t *graph, const node_t *node)
{
  if (graph == NULL || node == NULL)
    return NULL;

  int_t index = graph_node_index (graph, node);
  if (index < 0)
    return NULL;

  return fz_ref_at (graph->buffers, index, list_t);
}

/* Prepare GRAPH to render NFRAMES frames.  */
uint_t
fz_graph_prepare (graph_t *graph, size_t nframes)
{
  if (graph == NULL)
    return EINVAL;

  uint_t i;
  size_t nnodes = fz_len (graph->nodes);
  for (i = 0; i < nnodes; ++i)
    {
      fz_clear (fz_ref_at (graph->buffers, i, list_t), nframes);
      fz_node_prepare (fz_ref_at (graph->nodes, i, node_t), nframes);
      fz_val_at (graph->flags, i, flags_t) &= ~GRAPH_NODE_RENDERED;
    }

  return 0;
}

/* Render NODE into internal buffer of GRAPH using REQUEST.  */
static int_t
graph_node_render (graph_t *graph, node_t *node,
                   const request_t *request)
{
  int_t err;
  int_t index = graph_node_index (graph, node);
  flags_t *flags = fz_ref_at (graph->flags, index, flags_t);
  list_t *buffer = fz_ref_at (graph->buffers, index, list_t);
  if (*flags & GRAPH_NODE_RENDERED)
    return fz_len (buffer); /* NODE has already been rendered.  */

  /* Mix source outputs into NODEs buffer.  */
  real_t *frames = fz_list_data (buffer);
  size_t nframes = fz_len (buffer);
  uint_t srcidx;
  size_t nnodes = fz_len (graph->nodes);
  for (srcidx = 0; srcidx < nnodes; ++srcidx)
    {
      if (srcidx == (uint_t) index)
        continue;

      node_t *source = fz_ref_at (graph->nodes, srcidx, node_t);
      real_t *mix = graph_edge_ptr (graph, source, node);
      if (mix == NULL || *mix <= 0)
        continue; /* SOURCE is not a source of NODE (or MIX = 0).  */

      /* Render SOURCE.  */
      err = graph_node_render (graph, source, request);
      if (err < 0)
        return err; /* Relay failed render.  */

      uint_t frame;
      size_t nsrcframes = (size_t) err < nframes
        ? (size_t) err : nframes;
      real_t *srcframes = fz_list_data (fz_ref_at (graph->buffers,
                                                   srcidx, list_t));
      for (frame = 0; frame < nsrcframes; ++frame)
        frames[frame] += srcframes[frame] * *mix;
    }

  /* Render NODE.  */
  err = fz_node_render (node, buffer, request);
  if (err >= 0)
    *flags |= GRAPH_NODE_RENDERED;

  return err;
}

/* Render GRAPH using REQUEST.  */
int_t
fz_graph_render (graph_t *graph, const request_t *request)
{
  if (graph == NULL)
    return -EINVAL;

  int_t nrendered = 0;
  uint_t index;
  size_t nnodes = fz_len (graph->nodes);
  for (index = 0; index < nnodes; ++index)
    {
      node_t *node = fz_ref_at (graph->nodes, index, node_t);
      if (graph_node_is_sink (graph, node))
        {
          int_t err = graph_node_render (graph, node, request);
          if (err <= 0)
            return err;
          else if (err < nrendered || nrendered == 0)
            nrendered = err;
        }
    }

  return nrendered;
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
