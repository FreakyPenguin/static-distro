#!/bin/sh
set -e
cd ${PKG_NAME}-*/
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}" PREFIX="${PKG_DIR}"
