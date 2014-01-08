/* Header file defining ADSR modulator interface.
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

#ifndef FZ_ADSR_H
#define FZ_ADSR_H 1

#include "class.h"

__BEGIN_DECLS

#define ADSR_STATE_SILENT 0
#define ADSR_STATE_ATTACK 1
#define ADSR_STATE_DECAY 2
#define ADSR_STATE_SUSTAIN 3
#define ADSR_STATE_RELEASE 4

typedef struct adsr_s adsr_t;

extern const class_t *adsr_c;

__END_DECLS

#endif /* ! FZ_ADSR_H */
