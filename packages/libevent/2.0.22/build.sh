#!/bin/sh
set -e
mkdir build && cd build
../${PKG_NAME}-${PKG_VERSION}-stable/configure --prefix="${PKG_DIR}" \
    --oldincludedir=${PKG_DIR}/include --infodir=${PKG_DIR}/info \
    --localedir=${PKG_DIR}/locale --mandir=${PKG_DIR}/man --docdir=${PKG_DIR}/doc \
    --enable-static --disable-shared
make -j8
make install DESTDIR="${PKG_INSTDIR}"
