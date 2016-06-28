#!/bin/sh

set -e

repo="`pwd`/../packages/src/binutils-gdb/"
prefix="`(mkdir -p bs_prefix && cd bs_prefix && pwd)`"

rm -rf build-bs-binutils
mkdir build-bs-binutils
cd build-bs-binutils
git clone $repo src
cd src
git checkout static-bootstrap
cd ..

mkdir build
cd build
../src/configure --prefix="$prefix" --target=x86_64-linux-musl \
    --with-sysroot="$prefix" --disable-werror
make -j8
make install
