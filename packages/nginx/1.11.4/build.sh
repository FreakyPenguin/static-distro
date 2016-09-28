#!/bin/sh
set -e
cd "${PKG_NAME}-${PKG_VERSION}"
./configure --prefix="${PKG_DIR}" --sbin-path="${PKG_DIR}/bin/nginx" \
    --with-ld-opt=-static --with-cc=gcc
make -j8
make install DESTDIR="${PKG_INSTDIR}"
