/* Form node implementation.
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

#include <math.h>
#include "form.h"
#include "node.h"
#include "private-node.h"
#include "list.h"

#define SHAPE_SIZE 1024

/* Form class struct.  */
typedef struct form_s
{
  node_t __parent;
  list_t *shape;
  real_t pointer;
  real_t step;
} form_t;

/* Form node renderer.  */
static int_t
form_render (node_t *node)
{
  form_t *form = (form_t *) node;
  uint_t i = 0;
  size_t num_frames = fz_len (node->framebuf);
  size_t shape_size = fz_len (form->shape);

  if (shape_size == 0)
    return 0;

  for (; i < num_frames; ++i)
    {
      fz_val_at (node->framebuf, i, real_t)
        = fz_val_at (form->shape,
                     (uint_t) (form->pointer * shape_size),
                     real_t);

      form->pointer += form->step;
      while (form->pointer >= 1.)
        form->pointer -= 1.;
    }

  return num_frames;
}

/* Form constructor.  */
static ptr_t
form_constructor (ptr_t ptr, va_list *args)
{
  form_t *self = (form_t *)
    ((const class_t *) node_c)->construct (ptr, args);
  uint_t i;

  self->__parent.render = form_render;
  self->shape = fz_new_simple_vector (real_t);
  self->pointer = 0;
  self->step = 0.009977324; /* 440 Hz in 44100.  */

  fz_clear (self->shape, SHAPE_SIZE);
  for (; i < SHAPE_SIZE; ++i)
    fz_val_at (self->shape, i, real_t)
      = (fabs ((((real_t) i * 4) / SHAPE_SIZE) - 2) - 1) * -1;

  return self;
}

/* Form destructor.  */
static ptr_t
form_destructor (ptr_t ptr)
{
  form_t *self = (form_t *)
    ((const class_t *) node_c)->destruct (ptr);
  fz_del (self->shape);
  return self;
}

/* `form_c' class descriptor.  */
static const class_t _form_c = {
  sizeof (form_t),
  form_constructor,
  form_destructor,
  NULL,
  NULL,
  NULL
};

const class_t *form_c = &_form_c;
