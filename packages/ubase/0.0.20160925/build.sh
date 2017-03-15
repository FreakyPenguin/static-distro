#!/bin/sh
set -e
cd ${PKG_NAME}-*/
make $MAKE_JOBS all install PREFIX="${PKG_DIR}" MANPREFIX="${PKG_DIR}/man" \
    CC=gcc LDFLAGS="-s -static" DESTDIR="${PKG_INSTDIR}"
