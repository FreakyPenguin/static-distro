#!/bin/sh
set -e -x
ls
cd ${PKG_NAME}-*/
./configure --prefix="${PKG_DIR}" \
    --libdir=${PKG_DIR}/lib --with-sysdeps=/lib/skalibs/sysdeps \
    --disable-shared --enable-static-libc
make $MAKE_JOBS
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"
