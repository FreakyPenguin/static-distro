#!/bin/sh
set -e
cd ${PKG_NAME}-*/
./configure --prefix="${PKG_DIR}" --static
make -j8
make install DESTDIR="${PKG_INSTDIR}"
mv "${PKG_INSTDIR}/${PKG_DIR}/share/man" "${PKG_INSTDIR}/${PKG_DIR}/"
rm -r "${PKG_INSTDIR}/${PKG_DIR}/share"
rm -r "${PKG_INSTDIR}/${PKG_DIR}/lib/pkgconfig"
