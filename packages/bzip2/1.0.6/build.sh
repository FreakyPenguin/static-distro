#!/bin/sh
set -e
cd ${PKG_NAME}-*/
make install DESTDIR="${PKG_INSTDIR}" PREFIX="${PKG_DIR}"
