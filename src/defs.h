/* Misc defs and typedefs.
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

#ifndef FZ_DEFS_H
#define FZ_DEFS_H 1

#include <stdlib.h>
#include "config.h"

/* __BEGIN_DECLS is already defined in glibc.  */
#ifndef __BEGIN_DECLS
# ifdef __cplusplus
#  define __BEGIN_DECLS extern "C" {
#  define __END_DECLS }
# else
#  define __BEGIN_DECLS
#  define __END_DECLS
# endif
#endif

/* Define some types.  */
typedef void * ptr_t;
typedef double real_t;
typedef unsigned long long int flags_t;

#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
typedef int32_t int_t;
typedef uint32_t uint_t;
#elif HAVE_STDINT_H
# include <stdint.h>
typedef int32_t int_t;
typedef uint32_t uint_t;
#else
typedef int int_t;
typedef unsigned int uint_t;
#endif

#ifdef HAVE_STDBOOL_H
# include <stdbool.h>
#else
# ifndef HAVE__BOOL
#  ifdef __cplusplus
typedef bool _Bool;
#  else
#   define _Bool signed char
#  endif
# endif
#endif

#define bool_t _Bool
#define FALSE 0
#define TRUE 1

#endif /* ! FZ_DEFS_H */
