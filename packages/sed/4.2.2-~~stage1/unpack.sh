#!/bin/sh
tar xf ${PKG_NAME}-*.tar.*
cd ${PKG_NAME}-*
patch -p1 < ../${PKG_NAME}-musl.patch