#!/bin/sh

set -e

repo="`pwd`/../packages/src/sbase"
prefix=/packages/sbase/bootstrap/

rm -rf build-sbase
mkdir -p build-sbase
cd build-sbase
git clone $repo src

rootdir="`pwd`/root"

cd src
git checkout static-bootstrap
make -j8
make install DESTDIR=$rootdir
rm $rootdir/$prefix/bin/strings
