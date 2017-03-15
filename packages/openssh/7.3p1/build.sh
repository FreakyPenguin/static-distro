#!/bin/sh
set -e
mkdir build && cd build
../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
    --infodir=${PKG_DIR}/info --localedir=${PKG_DIR}/locale \
    --mandir=${PKG_DIR}/man --docdir=${PKG_DIR}/doc --sbindir=${PKG_DIR}/bin \
    --enable-static --disable-shared LDFLAGS=-static
make $MAKE_JOBS
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"
