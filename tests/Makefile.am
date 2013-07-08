# Process this file with automake to produce Makefile.in.
TESTS = check_malloc
check_PROGRAMS = check_malloc
check_malloc_SOURCES = check_malloc.c $(top_builddir)/src/malloc.h
check_malloc_CFLAGS = @CHECK_CFLAGS@
check_malloc_LDADD = $(top_builddir)/src/libfreeztile.la @CHECK_LIBS@
