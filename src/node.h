/* Header file exposing node class interface.
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

#ifndef FZ_NODE_H
#define FZ_NODE_H 1

#include "class.h"

__BEGIN_DECLS

typedef struct node_s node_t;

extern int_t fz_node_fork (node_t *, node_t *);
extern int_t fz_node_join (node_t *, node_t *);

extern const class_t *node_c;

__END_DECLS

#endif /* ! FZ_NODE_H */
