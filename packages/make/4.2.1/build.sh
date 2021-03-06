#!/bin/sh
set -e
docprefix=/packages/${PKG_NAME}-doc/${PKG_VERSION}
mkdir build && cd build
../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
    --oldincludedir=${PKG_DIR}/include --localedir=${PKG_DIR}/locale \
    --infodir=${docprefix}/info --mandir=${docprefix}/man \
    --docdir=${docprefix}/doc \
    --build=$ARCH_BUILD --target=$ARCH_TARGET --host=$ARCH_TARGET \
    --disable-shared --disable-load LDFLAGS="-static -s"
make $MAKE_JOBS
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"
rm -rf ${PKG_INSTDIR}/${docprefix}/info/dir \
    ${PKG_INSTDIR}/${PKG_DIR}/include
