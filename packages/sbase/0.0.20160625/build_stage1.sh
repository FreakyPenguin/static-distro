#!/bin/sh
set -e
cd ${PKG_NAME}-${PKG_VERSION}
make -j8 all install PREFIX="${PKG_DIR}" MANPREFIX="${PKG_DIR}/man" \
    CC=${ARCH_TARGET}-gcc AR=${ARCH_TARGET}-ar RALIB=${ARCH_TARGET}-ranlib \
    LDFLAGS="-s -static" DESTDIR="${PKG_INSTDIR}"
# remove some binaries
for b in strings grep sed ; do
    rm "${PKG_INSTDIR}/${PKG_DIR}/bin/$b"
done
