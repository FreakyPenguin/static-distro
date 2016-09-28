#!/bin/sh
tar xf ${PKG_NAME}-*.tar.*
cd ${PKG_NAME}-${PKG_VERSION}
patch -p1 < ../opensmtpd-musl-WAIT_MYPGRP.patch
patch -p1 < ../opensmtpd-libressl.patch
