#!/bin/sh
set -e
cd ${PKG_NAME}-*/

case $PKG_VERSION in
    *stage1)
        make $MAKE_JOBS all install PREFIX="${PKG_DIR}" MANPREFIX="${PKG_DIR}/man" \
            CC=${ARCH_TARGET}-gcc AR=${ARCH_TARGET}-ar RALIB=${ARCH_TARGET}-ranlib \
            LDFLAGS="-s -static" DESTDIR="${PKG_INSTDIR}"
        ;;
    *)
        make $MAKE_JOBS all install PREFIX="${PKG_DIR}" MANPREFIX="${PKG_DIR}/man" \
            CC=gcc LDFLAGS="-s -static" DESTDIR="${PKG_INSTDIR}"
        ;;
esac
# remove some binaries
for b in strings grep sed tar ; do
    rm "${PKG_INSTDIR}/${PKG_DIR}/bin/$b"
done
