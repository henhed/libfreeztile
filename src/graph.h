/* Header file exposing graph class interface.
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

#ifndef FZ_GRAPH_H
#define FZ_GRAPH_H 1

#include "node.h"
#include "class.h"

__BEGIN_DECLS

typedef struct graph_s graph_t;

extern bool_t fz_graph_has_node (const graph_t *, const node_t *);
extern uint_t fz_graph_add_node (graph_t *, node_t *);
extern uint_t fz_graph_del_node (graph_t *, node_t *);

extern const class_t *graph_c;

__END_DECLS

#endif /* ! FZ_GRAPH_H */
