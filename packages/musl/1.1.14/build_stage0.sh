#!/bin/sh
set -e
mkdir build && cd build
../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
    --build=$ARCH_HOST --host=$ARCH_TARGET --target=$ARCH_TARGET \
    --enable-optimize CROSS_COMPILE="$ARCH_TARGET-" CC="${ARCH_TARGET}-gcc"
make -j8
make install DESTDIR="${PKG_INSTDIR}"
