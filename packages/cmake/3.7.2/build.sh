#!/bin/sh
set -e
cd ${PKG_NAME}-*/
LDFLAGS="-static -s" ./bootstrap --prefix="${PKG_DIR}" \
    --datadir=${PKG_DIR}/share \
    --system-libs --no-system-jsoncpp --verbose \
    -- -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_USE_OPENSSL:BOOL=ON --debug-output --trace
make $MAKE_JOBS
make $MAKE_JOBS install DESTDIR="${PKG_INSTDIR}"
rm -rf ${PKG_INSTDIR}/${PKG_DIR}/doc
