#!/bin/sh
set -e
mkdir build && cd build
../${PKG_NAME}-${PKG_VERSION}/configure --prefix="${PKG_DIR}" \
    --target=$ARCH_TARGET --with-sysroot="${PKG_DIR}" --disable-werror
make -j8
make install DESTDIR="${PKG_INSTDIR}"
rm -f ${PKG_INSTDIR}/${PKG_DIR}/info/dir ${PKG_INSTDIR}/${PKG_DIR}/lib/charset.alias