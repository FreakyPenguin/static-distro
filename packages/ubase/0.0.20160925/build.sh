#!/bin/sh
set -e
docprefix=/packages/${PKG_NAME}-doc/${PKG_VERSION}
cd ${PKG_NAME}-*/
make $MAKE_JOBS all install PREFIX="${PKG_DIR}" MANPREFIX="${docprefix}/man" \
    CC=gcc LDFLAGS="-s -static" DESTDIR="${PKG_INSTDIR}"
