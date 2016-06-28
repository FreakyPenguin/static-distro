#!/bin/sh

set -e

repo="`pwd`/../packages/src/gcc/"
prefix="`(mkdir -p bs_prefix && cd bs_prefix && pwd)`"

# make sure prefix/usr/{lib,include} are symlinked to the dirs in prefix
mkdir -p $prefix/usr
if [ ! -L "$prefix/usr/lib" ] ; then
    ln -s $prefix/lib $prefix/usr/lib
fi
if [ ! -L "$prefix/usr/include" ] ; then
    ln -s $prefix/include $prefix/usr/include
fi


rm -rf build-bs-gcc-1
mkdir build-bs-gcc-1
cd build-bs-gcc-1
git clone $repo src
cd src
git checkout static-bootstrap
cd ..

mkdir build
cd build
CFLAGS="-O0 -g0" CXXFLAGS="-O0 -g0" ../src/configure --prefix="$prefix" \
    --target=x86_64-linux-musl --with-sysroot="$prefix" --with-newlib \
    --enable-languages=c --disable-libssp --disable-nls \
    --disable-libquadmath --disable-threads --disable-decimal-float \
    --disable-shared --disable-libmudflap --disable-libgomp \
    --disable-libatomic --disable-werror \
    --disable-multilib --with-multilib-list=
make -j8
make install

rm -rf "$prefix/lib/gcc/x86_64-linux-musl"/*/include-fixed/ \
    "$prefix/lib/gcc/x86_64-linux-musl"/*/include/stddef.h

