#!/bin/sh

set -e

repo="`pwd`/../packages/src/musl"
prefix="/packages/musl/bootstrap"
buildarch="`gcc -dumpmachine`"

rm -rf build-musl
mkdir -p build-musl
cd build-musl
git clone $repo src

rootdir="`pwd`/root"

cd src
git checkout static-bootstrap
./configure --prefix="$prefix" --build=$buildarch --host=x86_64-linux-musl \
    --target=x86_64-linux-musl -enable-optimize --syslibdir=$prefix/lib \
    CROSS_COMPILE="x86_64-linux-musl-" CC="x86_64-linux-musl-gcc"
make -j8
make install DESTDIR=$rootdir
