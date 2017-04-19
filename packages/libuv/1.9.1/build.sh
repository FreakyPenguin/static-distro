#!/bin/sh
set -e
mkdir build && cd build
../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
    --disable-shared LDFLAGS="-static -s"
make $MAKE_JOBS
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"
