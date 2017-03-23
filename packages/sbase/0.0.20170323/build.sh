#!/bin/sh
set -e
docprefix=/packages/${PKG_NAME}-doc/${PKG_VERSION}
cd ${PKG_NAME}-*/

case $PKG_VERSION in
    *stage1)
        make $MAKE_JOBS all install PREFIX="${PKG_DIR}" MANPREFIX="${docprefix}/man" \
            CC=${ARCH_TARGET}-gcc AR=${ARCH_TARGET}-ar RALIB=${ARCH_TARGET}-ranlib \
            LDFLAGS="-s -static" DESTDIR="${PKG_INSTDIR}"
        ;;
    *)
        make $MAKE_JOBS all install PREFIX="${PKG_DIR}" MANPREFIX="${docprefix}/man" \
            CC=gcc LDFLAGS="-s -static" DESTDIR="${PKG_INSTDIR}"
        ;;
esac

# Split out separate packages for these
for b in strings grep sed tar ; do
    dir="${PKG_INSTDIR}/packages/${PKG_NAME}-${b}/${PKG_VERSION}/bin"
    mkdir -p "$dir"
    mv "${PKG_INSTDIR}/${PKG_DIR}/bin/$b" "$dir/"
done
