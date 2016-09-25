#!/bin/sh
set -e
mkdir build && cd build
../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
    --target=$ARCH_TARGET --with-sysroot="${PKG_DIR}" \
    --enable-languages=c,c++ --disable-libssp --disable-nls \
    --disable-libquadmath --disable-threads --disable-decimal-float \
    --disable-shared --disable-libmudflap --disable-libgomp \
    --disable-libatomic --disable-werror  --disable-libsanitizer \
    --disable-multilib --with-multilib-list=

make -j8
make install DESTDIR="${PKG_INSTDIR}"
rm -f ${PKG_INSTDIR}/${PKG_DIR}/info/dir ${PKG_INSTDIR}/${PKG_DIR}/lib/charset.alias
