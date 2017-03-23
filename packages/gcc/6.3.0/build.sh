#!/bin/sh
set -e
docprefix=/packages/${PKG_NAME}-doc/${PKG_VERSION}
mkdir build && cd build

../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
    --oldincludedir=${PKG_DIR}/include --localedir=${PKG_DIR}/locale \
    --infodir=${docprefix}/info --mandir=${docprefix}/man \
    --docdir=${docprefix}/doc \
    --build=$ARCH_BUILD --target=$ARCH_TARGET --host=$ARCH_TARGET \
    --enable-languages=c,c++ --disable-nls --disable-libmudflap \
    --disable-libsanitizer --disable-werror --disable-multilib \
    --enable-static --disable-shared --disable-host-shared \
    --disable-lto \
    --with-stage1-ldflags="-static-libstdc++ -static-libgcc -static" \
    --with-boot-ldflags="-static-libstdc++ -static-libgcc -static" \
    LDFLAGS=-static

make $MAKE_JOBS
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"
rm -f ${PKG_INSTDIR}/${docprefix}/info/dir \
    ${PKG_INSTDIR}/${PKG_DIR}/lib/charset.alias
