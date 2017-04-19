#!/bin/sh
tar xof ${PKG_NAME}-*.tgz
cd ${PKG_NAME}-*/
patch -p1 < ../musl-force.patch
