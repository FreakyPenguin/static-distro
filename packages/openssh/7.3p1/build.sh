#!/bin/sh
set -e
mkdir build && cd build
../${PKG_NAME}-${PKG_VERSION}/configure --prefix="${PKG_DIR}" \
    --infodir=${PKG_DIR}/info --localedir=${PKG_DIR}/locale \
    --mandir=${PKG_DIR}/man --docdir=${PKG_DIR}/doc --sbindir=${PKG_DIR}/bin \
    --enable-static --disable-shared LDFLAGS=-static
make -j8
make install DESTDIR="${PKG_INSTDIR}"
