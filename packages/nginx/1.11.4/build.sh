#!/bin/sh
set -e
cd ${PKG_NAME}-*/
./configure --prefix="${PKG_DIR}" --sbin-path="${PKG_DIR}/bin/nginx" \
    --with-ld-opt=-static --with-cc=gcc
make -j8
make install DESTDIR="${PKG_INSTDIR}"
