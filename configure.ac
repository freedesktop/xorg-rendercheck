#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT([rendercheck],
        1.0.2,
        [anholt@FreeBSD.org],
        rendercheck)

AC_CONFIG_SRCDIR([Makefile.am])
AM_INIT_AUTOMAKE([dist-bzip2])

AC_CONFIG_AUX_DIR(.)

AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CC

# Checks for pkg-config packages
PKG_CHECK_MODULES(RC, xrender)
AC_SUBST(RC_CFLAGS)
AC_SUBST(RC_LIBS)

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC

AC_OUTPUT([Makefile])
