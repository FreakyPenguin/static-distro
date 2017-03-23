#!/bin/sh
set -e
docprefix=/packages/${PKG_NAME}-doc/${PKG_VERSION}
devprefix=/packages/${PKG_NAME}-dev/${PKG_VERSION}
mkdir build && cd build

case $PKG_VERSION in
    *stage1)
        ../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
            --oldincludedir=${devprefix}/include --libdir=${devprefix}/lib \
            --includedir=${devprefix}/include \
            --localedir=${PKG_DIR}/locale --mandir=${docprefix}/man \
            --docdir=${docprefix}/doc --infodir=${docprefix}/info \
            --build=$ARCH_BUILD --target=$ARCH_TARGET --host=$ARCH_TARGET \
            --disable-werror --disable-nls --disable-gdb \
            --disable-libdecnumber --disable-readline --disable-sim
        make configure-host LIB_PATH=/lib
        make $MAKE_JOBS LDFLAGS=-all-static LIB_PATH=/lib
        make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"
        ;;

    *)
        ../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
            --oldincludedir=${devprefix}/include --libdir=${devprefix}/lib \
            --includedir=${devprefix}/include \
            --localedir=${PKG_DIR}/locale --mandir=${docprefix}/man \
            --docdir=${docprefix}/doc --infodir=${docprefix}/info \
            --disable-werror --disable-shared \
            --disable-nls LDFLAGS=-static
        make $MAKE_JOBS
        make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"
        ;;
esac
rm -f ${PKG_INSTDIR}/${docprefix}/info/dir \
    ${PKG_INSTDIR}/${devprefix}/lib/charset.alias
