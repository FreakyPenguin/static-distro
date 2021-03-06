#!/bin/sh
set -e
cd ${PKG_NAME}-*/
./configure --prefix="${PKG_DIR}" --sbin-path="${PKG_DIR}/bin/nginx" \
    --with-ld-opt="-static -s" --with-cc=gcc --with-cc-opt=-Wno-error \
    --with-http_ssl_module --with-http_realip_module --with-ipv6
make $MAKE_JOBS
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"
