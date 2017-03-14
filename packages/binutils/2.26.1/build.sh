#!/bin/sh
set -e
mkdir build && cd build

case $PKG_VERSION in
    *stage1)
        ../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
            --oldincludedir=${PKG_DIR}/include --infodir=${PKG_DIR}/info \
            --localedir=${PKG_DIR}/locale --mandir=${PKG_DIR}/man \
            --docdir=${PKG_DIR}/doc \
            --build=$ARCH_BUILD --target=$ARCH_TARGET --host=$ARCH_TARGET \
            --disable-werror --disable-nls --disable-gdb \
            --disable-libdecnumber --disable-readline --disable-sim
        make configure-host LIB_PATH=/lib
        make -j8 LDFLAGS=-all-static LIB_PATH=/lib
        make install DESTDIR="${PKG_INSTDIR}"
        ;;

    *)
        ../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
            --oldincludedir=${PKG_DIR}/include --infodir=${PKG_DIR}/info \
            --localedir=${PKG_DIR}/locale --mandir=${PKG_DIR}/man \
            --docdir=${PKG_DIR}/doc --disable-werror --disable-shared \
            --disable-nls LDFLAGS=-static
        make -j8
        make install DESTDIR="${PKG_INSTDIR}"
        ;;
esac
rm -f ${PKG_INSTDIR}/${PKG_DIR}/info/dir ${PKG_INSTDIR}/${PKG_DIR}/lib/charset.alias
