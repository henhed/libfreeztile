/* Private header file exposing mod class struct for subclassing.
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

#ifndef FZ_PRIV_MOD_H
#define FZ_PRIV_MOD_H 1

#include "mod.h"
#include "class.h"
#include "list.h"

__BEGIN_DECLS

/* Modulator class struct.  */
struct mod_s
{
  const class_t *__class;
  list_t *stepbuf;
};

__END_DECLS

#endif /* ! FZ_PRIV_MOD_H */
