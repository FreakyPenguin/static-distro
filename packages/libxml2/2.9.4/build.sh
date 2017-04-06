#!/bin/sh
set -e
docprefix=/packages/${PKG_NAME}-doc/${PKG_VERSION}
binprefix=/packages/${PKG_NAME}-bin/${PKG_VERSION}
mkdir build && cd build
../${PKG_NAME}-*/configure --prefix="${PKG_DIR}" \
    --oldincludedir=${PKG_DIR}/include --localedir=${PKG_DIR}/locale \
    --bindir=${binprefix}/bin \
    --infodir=${docprefix}/info --mandir=${docprefix}/man \
    --docdir=${docprefix}/doc \
    --disable-werror --disable-shared
make $MAKE_JOBS
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"

# not sure why the build system makes such a mess of this
docdir="${PKG_INSTDIR}/$docprefix"
mkdir -p ${docdir}/doc/
mv ${PKG_INSTDIR}/${PKG_DIR}/share/doc/libxml2*/* "${docdir}/doc/"
mv ${PKG_INSTDIR}/${PKG_DIR}/share/gtk-doc/html/* "${docdir}/doc/html/"
rm -rf ${PKG_INSTDIR}/${PKG_DIR}/share/{doc,gtk-doc}
