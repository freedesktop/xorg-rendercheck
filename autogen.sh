#! /bin/sh
automake --foreign || exit 1
autoconf || exit 1
./configure --enable-maintainer-mode "$@"
