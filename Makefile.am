bin_PROGRAMS = rendercheck
man_MANS = rendercheck.1

rendercheck_SOURCES = \
	main.c \
	ops.c \
	rendercheck.h \
	tests.c

rendercheck_CFLAGS = $(RC_CFLAGS)
rendercheck_LDADD = $(RC_LIBS)

EXTRA_DIST = \
	autogen.sh \
	${man_MANS}
