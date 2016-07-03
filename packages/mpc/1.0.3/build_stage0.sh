#!/bin/sh
set -e
mkdir build && cd build
# not sure why I need to manually override cc here
../${PKG_NAME}-${PKG_VERSION}/configure --prefix="${PKG_DIR}" \
    --build=$ARCH_HOST --host=$ARCH_TARGET --target=$ARCH_TARGET \
    --disable-werror CC=${ARCH_TARGET}-gcc
make -j8
make install DESTDIR="${PKG_INSTDIR}"
