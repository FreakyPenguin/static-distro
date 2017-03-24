#!/bin/sh
set -e

cd ${PKG_NAME}-*/
sh Configure \
    -Duselargefiles \
    -Uuse64bitint \
    -Dusemymalloc=n \
    -Uusedl \
    -Uusethreads \
    -Uuseithreads \
    -Uusemultiplicity \
    -Uusesfio \
    -Uuseshrplib \
    -Uinstallusrbinperl \
    -Dcc="gcc" \
    -Dldflags="-s -static" \
    -Dlibs="-lm -lcrypt" \
    -Dprefix="$PKG_DIR" \
    -Dbin="$PKG_DIR/bin" \
    -Dprivlib="$PKG_DIR/lib" \
    -Darchlib="$PKG_DIR/lib" \
    -Uusevendorprefix \
    -Dsitelib="$PKG_DIR/lib" \
    -Dsitearch="$PKG_DIR/lib" \
    -Uman1dir \
    -Uman3dir \
    -Usiteman1dir \
    -Usiteman3dir \
    -Dpager=/usr/bin/less \
    -Duseperlio \
    -des

make $MAKE_JOBS

make $MAKE_JOBS install.perl DESTDIR="${PKG_INSTDIR}"
rm -f ${PKG_INSTDIR}/${PKG_DIR}/info/dir ${PKG_INSTDIR}/${PKG_DIR}/lib/charset.alias
