# Process this file with automake to produce Makefile.in.
TESTS = check_malloc check_class check_list check_node
check_PROGRAMS = check_malloc check_class check_list check_node
check_malloc_SOURCES = check_malloc.c $(top_builddir)/src/malloc.h
check_malloc_CFLAGS = @CHECK_CFLAGS@
check_malloc_LDADD = $(top_builddir)/src/libfreeztile.la @CHECK_LIBS@
check_class_SOURCES = check_class.c $(top_builddir)/src/class.h
check_class_CFLAGS = @CHECK_CFLAGS@
check_class_LDADD = $(top_builddir)/src/libfreeztile.la @CHECK_LIBS@
check_list_SOURCES = check_list.c $(top_builddir)/src/list.h
check_list_CFLAGS = @CHECK_CFLAGS@
check_list_LDADD = $(top_builddir)/src/libfreeztile.la @CHECK_LIBS@
check_node_SOURCES = check_node.c $(top_builddir)/src/node.h
check_node_CFLAGS = @CHECK_CFLAGS@
check_node_LDADD = $(top_builddir)/src/libfreeztile.la @CHECK_LIBS@
