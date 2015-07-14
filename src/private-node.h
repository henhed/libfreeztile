/* Private header file exposing node class struct.
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

#ifndef FZ_PRIV_NODE_H
#define FZ_PRIV_NODE_H 1

#include "node.h"
#include "class.h"
#include "list.h"
#include "map.h"

__BEGIN_DECLS

#define fz_node_modulate_snorm(node, slot, seed) \
  fz_node_modulate (node, slot, seed, -1,  1)

#define fz_node_modulate_unorm(node, slot, seed) \
  fz_node_modulate (node, slot, seed, 0,  1)

/* node class struct.  */
struct node_s
{
  const class_t *__class;
  map_t *mods;
  map_t *states;
  size_t state_size;
  void (*state_init) (node_t *, voice_t *, ptr_t);
  void (*state_free) (node_t *, voice_t *, ptr_t);
  int_t (*render) (node_t *, list_t *, const request_t *);
};

extern ptr_t fz_node_state (node_t *, const voice_t *);
extern ptr_t fz_node_modargs (const node_t *, uint_t);
extern const real_t * fz_node_modulate (node_t *, uint_t,
                                        real_t, real_t, real_t);

__END_DECLS

#endif /* ! FZ_PRIV_NODE_H */
