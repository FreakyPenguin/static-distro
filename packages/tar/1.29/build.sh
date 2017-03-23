#!/bin/sh
set -e
docprefix=/packages/${PKG_NAME}-doc/${PKG_VERSION}
docinst=${PKG_INSTDIR}/${docprefix}
mkdir build && cd build
FORCE_UNSAFE_CONFIGURE=1 ../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
    --oldincludedir=${PKG_DIR}/include --infodir=${docprefix}/info \
    --localedir=${PKG_DIR}/locale --mandir=${docprefix}/man \
    --docdir=${docprefix}/doc \
    --disable-shared LDFLAGS=-static
make $MAKE_JOBS
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"
rm -rf ${docinst}/info/dir ${PKG_INSTDIR}/${PKG_DIR}/lib/
