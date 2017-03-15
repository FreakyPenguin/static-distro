#!/bin/sh
set -e
mkdir build && cd build
../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
    --build=$ARCH_HOST --host=$ARCH_TARGET --target=$ARCH_TARGET \
    --disable-werror
make $MAKE_JOBS
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"
