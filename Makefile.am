bin_PROGRAMS = rendercheck
man_MANS = rendercheck.1

rendercheck_SOURCES = \
	main.c \
	ops.c \
	rendercheck.h \
	tests.c \
	t_blend.c \
	t_composite.c \
	t_dstcoords.c \
	t_fill.c \
	t_srccoords.c \
	t_tsrccoords.c

INCLUDES = $(RC_CFLAGS)
rendercheck_LDADD = $(RC_LIBS)

EXTRA_DIST = \
	autogen.sh \
	${man_MANS}
