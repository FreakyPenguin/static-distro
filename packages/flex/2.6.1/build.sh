#!/bin/sh
set -e
devprefix=/packages/${PKG_NAME}-dev/${PKG_VERSION}
docprefix=/packages/${PKG_NAME}-doc/${PKG_VERSION}
mkdir build && cd build
../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
    --oldincludedir=${devprefix}/include --includedir=${devprefix}/include \
    --libdir=${devprefix}/lib --localedir=${PKG_DIR}/locale \
    --infodir=${docprefix}/info --mandir=${docprefix}/man \
    --docdir=${docprefix}/doc \
    --disable-werror --disable-shared LDFLAGS="-static -s"
make $MAKE_JOBS
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"
rm -f ${PKG_INSTDIR}/${docprefix}/info/dir \
    ${PKG_INSTDIR}/${devprefix}/lib/charset.alias
