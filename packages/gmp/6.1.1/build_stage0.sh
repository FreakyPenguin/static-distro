#!/bin/sh
set -e
mkdir build && cd build
../${PKG_NAME}-${PKG_VERSION}/configure --prefix="${PKG_DIR}" \
    --build=$ARCH_HOST --host=$ARCH_TARGET --target=$ARCH_TARGET \
    --disable-werror
make -j8
make install DESTDIR="${PKG_INSTDIR}"
