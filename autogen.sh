#! /bin/sh
aclocal || exit 1
automake --foreign --add-missing || exit 1
autoconf || exit 1
./configure --enable-maintainer-mode "$@"
