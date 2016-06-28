#!/bin/sh

set -e

repo="`pwd`/../packages/src/gcc/"
prefix="/packages/gcc/bootstrap"
buildarch="`gcc -dumpmachine`"

rm -rf build-gcc
mkdir build-gcc
cd build-gcc
git clone $repo src
cd src
git checkout static-bootstrap
cd ..

rootdir="`pwd`/root"

mkdir build
cd build
../src/configure --prefix="$prefix" --build=$buildarch \
    --target=x86_64-linux-musl --host=x86_64-linux-musl \
    --enable-languages=c,c++ --disable-nls --disable-libmudflap \
    --disable-libsanitizer --disable-werror \
    --disable-shared --disable-host-shared --enable-static \
    LDFLAGS=-static
make -j8
make install DESTDIR=$rootdir

