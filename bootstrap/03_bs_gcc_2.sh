#!/bin/sh

set -e

repo="`pwd`/../packages/src/gcc/"
prefix="`(mkdir -p bs_prefix && cd bs_prefix && pwd)`"

rm -rf build-bs-gcc-2
mkdir build-bs-gcc-2
cd build-bs-gcc-2
git clone $repo src
cd src
git checkout static-bootstrap
cd ..

mkdir build
cd build
../src/configure --prefix="$prefix" \
    --target=x86_64-linux-musl --with-sysroot="$prefix" \
    --enable-languages=c,c++ --disable-libssp --disable-nls \
    --disable-libquadmath --disable-threads --disable-decimal-float \
    --disable-shared --disable-libmudflap --disable-libgomp \
    --disable-libatomic --disable-werror  --disable-libsanitizer \
    --disable-multilib --with-multilib-list=
make -j8
make install

rm -rf "$prefix/lib/gcc/x86_64-linux-musl"/*/include-fixed/ \
    "$prefix/lib/gcc/x86_64-linux-musl"/*/include/stddef.h

