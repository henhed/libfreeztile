/* Implementation of voice class interface.
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

#include "voice.h"
#include "class.h"

/* voice class struct.  */
struct voice_s
{
  const class_t *__class;
};

/* `voice_c' class descriptor.  */
static const class_t _voice_c = {
  sizeof (voice_t),
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

const class_t *voice_c = &_voice_c;
