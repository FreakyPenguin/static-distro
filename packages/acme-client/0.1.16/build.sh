#!/bin/sh
set -e
cd ${PKG_NAME}-*/
make $MAKE_JOBS install CC=gcc LDFLAGS="-static -s" FORCE_MUSL=1 \
    PREFIX="${PKG_DIR}" \
    MAN1DIR="/packages/${PKG_NAME}-doc/${PKG_VERSION}/man/man1" \
    DESTDIR="${PKG_INSTDIR}"
