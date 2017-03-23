#!/bin/sh
set -e
docprefix=/packages/${PKG_NAME}-doc/${PKG_VERSION}
cd ${PKG_NAME}
CC=${ARCH_TARGET}-gcc LDSTATIC=-static HAVE_STRLCPY=0 sh Build.sh -r -c lto
mkdir -p ${PKG_INSTDIR}/${PKG_DIR}/bin
mkdir -p ${PKG_INSTDIR}/${docprefix}/doc/examples
mkdir -p ${PKG_INSTDIR}/${docprefix}/man/man1
cp mksh ${PKG_INSTDIR}/${PKG_DIR}/bin/
ln -s mksh ${PKG_INSTDIR}/${PKG_DIR}/bin/sh
cp dot.mkshrc ${PKG_INSTDIR}/${docprefix}/doc/examples/
cp mksh.1 ${PKG_INSTDIR}/${docprefix}/man/man1/
