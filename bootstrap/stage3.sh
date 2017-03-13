#!/bin/sh
set -e

rootdir="`pwd`/.."
s2_pkgs="`pwd`/stage2_packages"
export PATH="$rootdir/tools:$PATH"


newns withpkgs -d "$rootdir/bootstrap/stage2_packages" -w "$rootdir" \
  -c "/work/bootstrap" \
  -p mksh -p sbase -p staticdistro-tools \
  -- ./stage3_inner.sh
