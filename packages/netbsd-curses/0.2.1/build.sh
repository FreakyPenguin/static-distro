#!/bin/sh
set -e
cd ${PKG_NAME}-*/
make $MAKE_JOBS \
    PREFIX="${PKG_DIR}" \
    DESTDIR="${PKG_INSTDIR}" \
    CC=gcc CFLAGS=-O2 LDFLAGS=-static \
    all-static install-static
