#!/bin/sh
set -e
mkdir build && cd build
../${PKG_NAME}-${PKG_VERSION}/configure --prefix="${PKG_DIR}" \
    --oldincludedir=${PKG_DIR}/include --infodir=${PKG_DIR}/info \
    --localedir=${PKG_DIR}/locale --mandir=${PKG_DIR}/man --docdir=${PKG_DIR}/doc \
    --build=$ARCH_BUILD --target=$ARCH_TARGET --host=$ARCH_TARGET \
    --disable-werror --disable-nls --disable-gdb --disable-libdecnumber \
    --disable-readline --disable-sim
make configure-host
make -j8 LDFLAGS=-all-static
make install DESTDIR="${PKG_INSTDIR}"
rm -f ${PKG_INSTDIR}/${PKG_DIR}/info/dir ${PKG_INSTDIR}/${PKG_DIR}/lib/charset.alias
