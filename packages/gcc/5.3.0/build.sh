#!/bin/sh
set -e
mkdir build && cd build
../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
    --oldincludedir=${PKG_DIR}/include --infodir=${PKG_DIR}/info \
    --localedir=${PKG_DIR}/locale --mandir=${PKG_DIR}/man --docdir=${PKG_DIR}/doc \
    --build=$ARCH_BUILD --target=$ARCH_TARGET --host=$ARCH_TARGET \
    --enable-languages=c,c++ --disable-nls --disable-libmudflap \
    --disable-libsanitizer --disable-werror --disable-multilib \
    --enable-static --disable-shared --disable-host-shared \
    --disable-lto \
    LDFLAGS=-static
make -j8
make install-no-fixedincludes DESTDIR="${PKG_INSTDIR}"
rm -f ${PKG_INSTDIR}/${PKG_DIR}/info/dir ${PKG_INSTDIR}/${PKG_DIR}/lib/charset.alias
