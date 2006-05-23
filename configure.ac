#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([rendercheck],
        1.1,
        [anholt@FreeBSD.org],
        rendercheck)

AC_CONFIG_SRCDIR([Makefile.am])
AM_INIT_AUTOMAKE([dist-bzip2])

AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CC

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

if  test "x$GCC" = xyes ; then
    RC_CFLAGS="-Wall $RC_CFLAGS"
fi

AC_SUBST(RC_CFLAGS)
AC_SUBST(RC_LIBS)

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([err.h])

AC_OUTPUT([Makefile])
