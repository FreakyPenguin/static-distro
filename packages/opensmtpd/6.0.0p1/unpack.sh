#!/bin/sh
tar xof ${PKG_NAME}-*.tar.*
cd ${PKG_NAME}-*/
patch -p1 < ../opensmtpd-musl-WAIT_MYPGRP.patch
patch -p1 < ../opensmtpd-libressl.patch
