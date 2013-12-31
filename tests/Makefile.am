# Process this file with automake to produce Makefile.in.
TESTS =          \
    check_malloc \
    check_class  \
    check_list   \
    check_voice  \
    check_mod    \
    check_node   \
    check_form
check_PROGRAMS = \
    check_malloc \
    check_class  \
    check_list   \
    check_voice  \
    check_mod    \
    check_node   \
    check_form
check_malloc_SOURCES = check_malloc.c $(top_builddir)/src/malloc.h
check_malloc_CFLAGS = @CHECK_CFLAGS@
check_malloc_LDADD = $(top_builddir)/src/libfreeztile.la @CHECK_LIBS@
check_class_SOURCES = check_class.c $(top_builddir)/src/class.h
check_class_CFLAGS = @CHECK_CFLAGS@
check_class_LDADD = $(top_builddir)/src/libfreeztile.la @CHECK_LIBS@
check_list_SOURCES = check_list.c $(top_builddir)/src/list.h
check_list_CFLAGS = @CHECK_CFLAGS@
check_list_LDADD = $(top_builddir)/src/libfreeztile.la @CHECK_LIBS@
check_voice_SOURCES = check_voice.c $(top_builddir)/src/voice.h
check_voice_CFLAGS = @CHECK_CFLAGS@
check_voice_LDADD = $(top_builddir)/src/libfreeztile.la @CHECK_LIBS@
check_mod_SOURCES = check_mod.c $(top_builddir)/src/mod.h
check_mod_CFLAGS = @CHECK_CFLAGS@
check_mod_LDADD = $(top_builddir)/src/libfreeztile.la @CHECK_LIBS@
check_node_SOURCES = check_node.c $(top_builddir)/src/node.h
check_node_CFLAGS = @CHECK_CFLAGS@
check_node_LDADD = $(top_builddir)/src/libfreeztile.la @CHECK_LIBS@
check_form_SOURCES = check_form.c $(top_builddir)/src/form.h
check_form_CFLAGS = @CHECK_CFLAGS@
check_form_LDADD = $(top_builddir)/src/libfreeztile.la @CHECK_LIBS@
