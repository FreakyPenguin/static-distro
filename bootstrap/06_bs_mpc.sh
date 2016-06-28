#!/bin/sh

set -e

repo="`pwd`/../packages/src/mpc/"
prefix="`(mkdir -p bs_prefix && cd bs_prefix && pwd)`"
buildarch="`gcc -dumpmachine`"

rm -rf build-bs-mpc
mkdir build-bs-mpc
cd build-bs-mpc
git clone $repo src
cd src
git checkout static-bootstrap
autoreconf -i
cd ..

mkdir build
cd build
# TODO: figure out why we need to override CC
../src/configure --prefix="$prefix" --build=$buildarch \
    --target=x86_64-linux-musl --host=x86_64-linux-musl --disable-werror \
    CC=x86_64-linux-musl-gcc
make -j8
make install
