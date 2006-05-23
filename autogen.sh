#! /bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

ORIGDIR=`pwd`
cd $srcdir

aclocal || exit 1
automake --foreign --add-missing || exit 1
autoconf || exit 1

cd $ORIGDIR || exit $?

$srcdir/configure --enable-maintainer-mode "$@"
