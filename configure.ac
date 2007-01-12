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

if test "x$GCC" = "xyes"; then
       GCC_WARNINGS1="-Wall -Wpointer-arith -Wstrict-prototypes"
       GCC_WARNINGS2="-Wmissing-prototypes -Wmissing-declarations"
       GCC_WARNINGS3="-Wnested-externs -fno-strict-aliasing"
       GCC_WARNINGS="$GCC_WARNINGS1 $GCC_WARNINGS2 $GCC_WARNINGS3"
       if test "x$WERROR" = "xyes"; then
               GCC_WARNINGS="${GCC_WARNINGS} -Werror"
       fi
       RC_CFLAGS="$RC_CFLAGS $GCC_WARNINGS"
fi

AC_SUBST(RC_CFLAGS)
AC_SUBST(RC_LIBS)

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([err.h])

AC_OUTPUT([Makefile])
