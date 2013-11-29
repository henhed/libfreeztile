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

#include "node.h"
#include "defs.h"
#include "class.h"

/* node class struct.  */
struct node_s
{
  const class_t *__class;
};

/* `node_c' class descriptor.  */
static const class_t _node_c = {
  sizeof (node_t),
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

const class_t *node_c = &_node_c;
