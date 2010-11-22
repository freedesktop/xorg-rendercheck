#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.60)
AC_INIT([rendercheck],
        [1.4],
        [https://bugs.freedesktop.org/enter_bug.cgi?product=xorg],
        [rendercheck])

AC_CONFIG_SRCDIR([Makefile.am])
AM_INIT_AUTOMAKE([foreign dist-bzip2])

AM_MAINTAINER_MODE

# Require X.Org macros 1.8 or later for MAN_SUBSTS set by XORG_MANPAGE_SECTIONS
m4_ifndef([XORG_MACROS_VERSION],
          [m4_fatal([must install xorg-macros 1.8 or later before running autoconf/autogen])])
XORG_MACROS_VERSION(1.8)
XORG_DEFAULT_OPTIONS

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([err.h])

# Checks for pkg-config packages
PKG_CHECK_MODULES(RC, [xrender x11])

AC_CONFIG_FILES([Makefile
                 man/Makefile])

AC_OUTPUT
