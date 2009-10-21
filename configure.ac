#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([rendercheck],
        1.3,
        [anholt@FreeBSD.org],
        rendercheck)

AC_CONFIG_SRCDIR([Makefile.am])
AM_INIT_AUTOMAKE([foreign dist-bzip2])

AM_MAINTAINER_MODE

# Require xorg-macros: XORG_CWARNFLAGS, XORG_CHANGELOG
m4_ifndef([XORG_MACROS_VERSION], [AC_FATAL([must install xorg-macros 1.2 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.2)

AC_PROG_CC

XORG_CWARNFLAGS

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([err.h])

AC_ARG_ENABLE(werror,        AS_HELP_STRING([--enable-werror],
				  [Treat warnings as errors (default: disabled)]),
			        [WERROR=$enableval], [WERROR=no])

# Checks for pkg-config packages
PKG_CHECK_MODULES(RC, xrender,
		  [xrender_found_with_pkgconfig=yes],
		  [xrender_found_with_pkgconfig=no])

if test "$xrender_found_with_pkgconfig" = "no"
then
	AC_PATH_XTRA
	if test "x$no_x" = "xyes"
	then
		AC_MSG_ERROR([X is required, but it was either disabled or not found.])
	fi

	RC_CFLAGS="$X_CFLAGS"
	RC_LIBS="$X_LIBS -lXrender -lXext -lX11"
fi

if test "x$GCC" = "xyes" -a "x$WERROR" = "xyes"; then
	CWARNFLAGS="$CWARNFLAGS -Werror"
fi
RC_CFLAGS="$CWARNFLAGS $RC_CFLAGS"
AC_SUBST(RC_CFLAGS)
AC_SUBST(RC_LIBS)

XORG_CHANGELOG

AC_OUTPUT([Makefile])
