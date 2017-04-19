#!/bin/sh
set -e
docprefix=/packages/${PKG_NAME}-doc/${PKG_VERSION}
devprefix=/packages/${PKG_NAME}-dev/${PKG_VERSION}
mkdir build && cd build
../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
    --oldincludedir=${devprefix}/include --includedir=${devprefix}/include \
    --libdir=${devprefix}/lib -datarootdir=${devprefix}/share \
    --infodir=${docprefix}/info --mandir=${docprefix}/man \
    --docdir=${docprefix}/doc \
    --disable-werror --disable-shared LDFLAGS="-static -s"
make $MAKE_JOBS
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"
