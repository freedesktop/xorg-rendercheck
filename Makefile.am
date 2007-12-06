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
	autogen.sh \
	${man_MANS}

EXTRA_DIST += ChangeLog
MAINTAINERCLEANFILES = ChangeLog

.PHONY: ChangeLog

ChangeLog:
	(GIT_DIR=$(top_srcdir)/.git git-log > .changelog.tmp && mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) || (touch ChangeLog; echo 'git directory not found: installing possibly empty changelog.' >&2)

dist-hook: ChangeLog
