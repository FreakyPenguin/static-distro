#!/bin/sh
set -e
docprefix=/packages/${PKG_NAME}-doc/${PKG_VERSION}
mkdir build && cd build
../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
    --oldincludedir=${PKG_DIR}/include --infodir=${docprefix}/info \
    --localedir=${PKG_DIR}/locale --mandir=${docprefix}/man \
    --docdir=${docprefix}/doc \
    --disable-shared LDFLAGS="-static -s -v"
make $MAKE_JOBS
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"
