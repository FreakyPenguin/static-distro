#!/bin/sh
set -e
mkdir build && cd build

case $PKG_VERSION in
    *stage1)
        ../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
            --build=$ARCH_HOST --host=$ARCH_TARGET --target=$ARCH_TARGET \
            --syslibdir="${PKG_DIR}/lib" --disable-shared \
            --enable-optimize CROSS_COMPILE="${ARCH_TARGET}-" \
            CC="${ARCH_TARGET}-gcc"
        ;;
    *)
        ../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
            --oldincludedir=${PKG_DIR}/include --infodir=${PKG_DIR}/info \
            --localedir=${PKG_DIR}/locale --mandir=${PKG_DIR}/man \
            --docdir=${PKG_DIR}/doc \
            --enable-optimize \
            --disable-werror --disable-shared LDFLAGS=-static
        ;;
esac
make -j8
make install DESTDIR="${PKG_INSTDIR}"
rm -f ${PKG_INSTDIR}/${PKG_DIR}/info/dir ${PKG_INSTDIR}/${PKG_DIR}/lib/charset.alias
