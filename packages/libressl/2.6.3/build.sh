#!/bin/sh
set -e
docprefix=/packages/${PKG_NAME}-doc/${PKG_VERSION}
binprefix=/packages/${PKG_NAME}-bin/${PKG_VERSION}
mkdir build && cd build
../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
    --oldincludedir=${PKG_DIR}/include --localedir=${PKG_DIR}/locale \
    --bindir=${binprefix}/bin \
    --infodir=${docprefix}/info --mandir=${docprefix}/man \
    --docdir=${docprefix}/doc --sysconfdir=${docprefix}/etc \
    --disable-werror --disable-shared LDFLAGS="-static -s"
make $MAKE_JOBS
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"
