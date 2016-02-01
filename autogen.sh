#! /bin/sh

srcdir=`dirname $0`
test -z "$srcdir" && srcdir=.

ORIGDIR=`pwd`
cd $srcdir

autoreconf -v --install || exit 1

cd $ORIGDIR || exit $?

$srcdir/configure --enable-maintainer-mode "$@"

git config --local --get format.subjectPrefix ||
    git config --local format.subjectPrefix "PATCH rendercheck"
