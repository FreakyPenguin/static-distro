#!/bin/sh
set -e
mkdir build && cd build
# not sure why I need to manually override cc here
../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
    --build=$ARCH_HOST --host=$ARCH_TARGET --target=$ARCH_TARGET \
    --disable-werror CC=${ARCH_TARGET}-gcc
make $MAKE_JOBS
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"
