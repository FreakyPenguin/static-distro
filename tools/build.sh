#!/bin/sh
make clean
make CC=${ARCH_TARGET}-gcc PREFIX="${PKG_DIR}" DESTDIR="${PKG_INSTDIR}" install
