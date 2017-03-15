#!/bin/sh
set -e
mkdir build && cd build
../musl-*/configure --prefix="${PKG_DIR}" \
    --syslibdir="${PKG_DIR}/lib" --disable-static --enable-shared \
    --enable-optimize
make $MAKE_JOBS lib/libc.so
mkdir -p "${PKG_INSTDIR}/${PKG_DIR}/lib"
cp lib/libc.so "${PKG_INSTDIR}/${PKG_DIR}/lib"
ln -s "${PKG_DIR}/lib/libc.so" \
    "${PKG_INSTDIR}/${PKG_DIR}/lib/ld-musl-x86_64.so"
#make install DESTDIR="${PKG_INSTDIR}"
