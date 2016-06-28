#!/bin/sh

set -e

repo="`pwd`/../packages/src/mksh"

rm -rf build-mksh
mkdir -p build-mksh
cd build-mksh
git clone $repo src

rootdir="`pwd`/root"

cd src
git checkout static-bootstrap
CC=x86_64-linux-musl-gcc LDSTATIC=-static HAVE_STRLCPY=0 sh Build.sh -r -c lto
mkdir -p $rootdir/packages/mksh/bootstrap/bin
mkdir -p $rootdir/packages/mksh/bootstrap/doc/examples
mkdir -p $rootdir/packages/mksh/bootstrap/man/man1
cp mksh $rootdir/packages/mksh/bootstrap/bin/
cp dot.mkshrc $rootdir/packages/mksh/bootstrap/doc/examples/
cp mksh.1 $rootdir/packages/mksh/bootstrap/man/man1/
