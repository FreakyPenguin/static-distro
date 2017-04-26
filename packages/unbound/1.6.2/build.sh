#!/bin/sh
set -e
docprefix=/packages/${PKG_NAME}-doc/${PKG_VERSION}
devprefix=/packages/${PKG_NAME}-dev/${PKG_VERSION}
#mkdir build && cd build
cd ${PKG_NAME}-*/
./configure --prefix="${PKG_DIR}" \
    --sysconfdir=${docprefix}/etc --mandir=${docprefix}/man \
    --docdir=${docprefix}/doc --infodir=${docprefix}/info \
    --includedir=${devprefix}/include --libdir=${devprefix}/lib \
    --sbindir=${PKG_DIR}/bin \
    --enable-static --disable-shared LDFLAGS="-static -s"
make $MAKE_JOBS
make install DESTDIR="${PKG_INSTDIR}"
