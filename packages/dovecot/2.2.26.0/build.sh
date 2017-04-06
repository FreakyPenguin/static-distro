#!/bin/sh
set -e
docprefix=/packages/${PKG_NAME}-doc/${PKG_VERSION}
devprefix=/packages/${PKG_NAME}-dev/${PKG_VERSION}
#mkdir build && cd build
cd ${PKG_NAME}-*/
./configure --prefix="${PKG_DIR}" \
    --infodir=${PKG_DIR}/info --localedir=${PKG_DIR}/locale --sbindir=${PKG_DIR}/bin \
    --mandir=${docprefix}/man --docdir=${docprefix}/doc  --sysconfdir=${docprefix}/etc \
    --libdir=${devprefix}/lib --includedir=${devprefix}/include \
    --enable-static --disable-shared LDFLAGS="-static -s"
make $MAKE_JOBS
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"
