#!/bin/sh
tar xof ${PKG_NAME}-*.tar.*
cd ${PKG_NAME}-*/
patch -p1 < ../${PKG_NAME}-destdir.patch
