#!/bin/sh

set -e

repo="`pwd`/../packages/src/binutils-gdb/"
prefix="/packages/binutils/bootstrap"
buildarch="`gcc -dumpmachine`"

rm -rf build-binutils
mkdir build-binutils
cd build-binutils
git clone $repo src
cd src
git checkout static-bootstrap
cd ..

rootdir="`pwd`/root"

mkdir build
cd build
../src/configure --prefix="$prefix" --build=$buildarch \
    --target=x86_64-linux-musl --host=x86_64-linux-musl --disable-werror \
    --disable-nls --disable-gdb --disable-libdecnumber --disable-readline \
    --disable-sim
make configure-host
make -j8 LDFLAGS=-all-static
make install DESTDIR=$rootdir

# remove libraries and headers
rm -rf $rootdir/$prefix/lib $rootdir/$prefix/include
