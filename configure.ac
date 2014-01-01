# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([libfreeztile], [0.1.0.0], [bugs@freeztile.org])
AC_CONFIG_SRCDIR([src/malloc.c])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

# Set build strictness
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# automake 1.12 seems to require AM_PROG_AR, but older
# versions doesn't recognize it
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

# Set up libtool (used by src/Makefile.am)
LT_INIT()

# Determine a C compiler to use.
AC_PROG_CC()
AM_PROG_CC_C_O()

# Check for libcheck C unit testing library
PKG_CHECK_MODULES([CHECK], [check >= 0.9.4])

# Do we have the _Bool built-in?
AC_HEADER_STDBOOL()

# Output files
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_FILES([Makefile src/Makefile tests/Makefile])

AC_OUTPUT
