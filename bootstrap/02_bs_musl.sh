#!/bin/sh

set -e

repo="`pwd`/../packages/src/musl"
prefix="`(mkdir -p bs_prefix && cd bs_prefix && pwd)`"
buildarch="`gcc -dumpmachine`"

rm -rf build-bs-musl
git clone $repo build-bs-musl
cd build-bs-musl
git checkout static-bootstrap
./configure --prefix="$prefix" --build=$buildarch --host=x86_64-linux-musl \
    --target=x86_64-linux-musl -enable-optimize \
    CROSS_COMPILE="x86_64-linux-musl-" CC="x86_64-linux-musl-gcc"
make -j8
make install
