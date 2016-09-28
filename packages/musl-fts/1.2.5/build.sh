#!/bin/sh
set -e
mkdir build && cd build
# not sure why I need to manually override cc here
../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
    --infodir=${PKG_DIR}/info --localedir=${PKG_DIR}/locale \
    --mandir=${PKG_DIR}/man --docdir=${PKG_DIR}/doc \
    --enable-static --disable-shared
make -j8
make install DESTDIR="${PKG_INSTDIR}"
