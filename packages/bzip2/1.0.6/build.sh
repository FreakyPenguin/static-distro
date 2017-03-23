#!/bin/sh
set -e
docprefix=/packages/${PKG_NAME}-doc/${PKG_VERSION}
devprefix=/packages/${PKG_NAME}-dev/${PKG_VERSION}
cd ${PKG_NAME}-*/
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}" PREFIX="${PKG_DIR}"

# prepare doc package
mkdir -p ${PKG_INSTDIR}/${docprefix}
mv ${PKG_INSTDIR}/${PKG_DIR}/man ${PKG_INSTDIR}/${docprefix}/

# prepare dev package
mkdir -p ${PKG_INSTDIR}/${devprefix}
mv ${PKG_INSTDIR}/${PKG_DIR}/lib ${PKG_INSTDIR}/${PKG_DIR}/include \
    ${PKG_INSTDIR}/${devprefix}/
