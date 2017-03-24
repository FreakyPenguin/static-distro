#!/bin/sh
set -e
cd ${PKG_NAME}-*/
./configure --prefix="${PKG_DIR}" --sbin-path="${PKG_DIR}/bin/nginx" \
    --with-ld-opt="-static -s" --with-cc=gcc
make $MAKE_JOBS
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"
