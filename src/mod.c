/* Implementation of modulator class interface.
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

#include "mod.h"
#include "private-mod.h"
#include "class.h"
#include "list.h"

/* Modulator constructor.  */
static ptr_t
mod_constructor (ptr_t ptr, va_list *args)
{
  mod_t *self = (mod_t *) ptr;
  self->stepbuf = fz_new_simple_vector (real_t);
  return self;
}

/* Modulator destructor.  */
static ptr_t
mod_destructor (ptr_t ptr)
{
  mod_t *self = (mod_t *) ptr;
  fz_del (self->stepbuf);
  return self;
}

/* `mod_c' class descriptor.  */
static const class_t _mod_c = {
  sizeof (mod_t),
  mod_constructor,
  mod_destructor,
  NULL,
  NULL,
  NULL
};

const class_t *mod_c = &_mod_c;
