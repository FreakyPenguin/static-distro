#!/bin/sh

set -e

cd $1
rm -rf bin lib include

mkdir -p bin lib include
for f in packages/*/bootstrap/bin/* ; do
    ln -s /$f bin/`basename $f`
done
for f in packages/*/bootstrap/lib/* ; do
    ln -s /$f lib/`basename $f`
done
for f in packages/*/bootstrap/include/* ; do
    ln -s /$f include/`basename $f`
done
