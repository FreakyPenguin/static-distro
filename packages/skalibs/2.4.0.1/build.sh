#!/bin/sh
set -e -x
ls
cd ${PKG_NAME}-*/
./configure --prefix="${PKG_DIR}" \
    --libdir=${PKG_DIR}/lib \
    --disable-shared
make -j8
make install DESTDIR="${PKG_INSTDIR}"
