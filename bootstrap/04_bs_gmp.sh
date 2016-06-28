#!/bin/sh

set -e

repo="`pwd`/../packages/src/gmp/"
prefix="`(mkdir -p bs_prefix && cd bs_prefix && pwd)`"
buildarch="`gcc -dumpmachine`"

rm -rf build-bs-gmp
mkdir build-bs-gmp
cd build-bs-gmp
git clone $repo src
cd src
git checkout static-bootstrap
cd ..

mkdir build
cd build
../src/configure --prefix="$prefix" --build=$buildarch \
    --target=x86_64-linux-musl --host=x86_64-linux-musl --disable-werror
make -j8
make install
