bin_PROGRAMS = rendercheck
man_MANS = rendercheck.1

rendercheck_SOURCES = \
	main.c \
	ops.c \
	rendercheck.h \
	tests.c \
	t_blend.c \
	t_bug7366.c \
	t_composite.c \
	t_dstcoords.c \
	t_fill.c \
	t_gradient.c \
	t_repeat.c \
	t_srccoords.c \
	t_tsrccoords.c \
	t_tsrccoords2.c \
	t_triangles.c

INCLUDES = $(RC_CFLAGS)
rendercheck_LDADD = $(RC_LIBS)

EXTRA_DIST = \
	AUTHORS \
	COPYING \
	INSTALL \
	README \
	doc/AddingNewTests \
	doc/TODO \
	${man_MANS}

EXTRA_DIST += ChangeLog
MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog
