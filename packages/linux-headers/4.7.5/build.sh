#!/bin/sh
set -e
cd linux-${PKG_VERSION}
make headers_install ARCH=x86_64 \
    INSTALL_HDR_PATH="${PKG_INSTDIR}/${PKG_DIR}/"
