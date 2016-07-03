#!/bin/sh
set -x
set -e

s0_prefix="`pwd`/stage0_prefix"
distfiles="`pwd`/../distfiles"
packages="`pwd`/../packages"
buildparentdir="`pwd`/stage0_build"

export ARCH_BUILD="`gcc -dumpmachine`"
export ARCH_TARGET=x86_64-linux-musl
export PATH="$s0_prefix/bin:$PATH"

# prepare prefix dir
mkdir -p $s0_prefix
# make sure prefix/usr/{lib,include} are symlinked to the dirs in prefix
mkdir -p $s0_prefix/usr
if [ ! -L "$s0_prefix/usr/lib" ] ; then
    ln -s $s0_prefix/lib $s0_prefix/usr/lib
fi
if [ ! -L "$s0_prefix/usr/include" ] ; then
    ln -s $s0_prefix/include $s0_prefix/usr/include
fi

mkdir -p $buildparentdir

build_s0_pkg() {
    pkg="$1"
    pass="$2"

    # make sure it's not completed yet
    build_dir="${buildparentdir}/build-${pkg}-${pass}"
    if [ -f "${build_dir}/.done" ] ; then
        echo "stage0: Skipping $pkg pass $pass"
        return
    fi

    # figure out version
    phys_path="`cd ${packages}/${pkg}/stage0 && pwd -P`"
    ver="`basename $phys_path`"

    echo "stage0: Building $pkg pass $pass"

    # prepare build directory
    rm -rf "$build_dir"
    cp -r "$phys_path" "$build_dir"
    cd "$build_dir"

    # get distfiles
    while read f ; do
        cp "${distfiles}/${f}" .
    done < srcs

    export PKG_NAME="$pkg"
    export PKG_VERSION="$ver"
    export PKG_DIR="$s0_prefix"
    export PKG_INSTDIR="/"

    # unpack
    ./unpack.sh >build.log 2>&1

    # build pass
    ./build_${pass}.sh >>build.log 2>&1

    touch "${build_dir}/.done"
    cd "${buildparentdir}"
}

build_s0_pkg binutils stage0
build_s0_pkg gcc stage0_1
build_s0_pkg musl stage0
build_s0_pkg gcc stage0_2
build_s0_pkg gmp stage0
build_s0_pkg mpfr stage0
build_s0_pkg mpc stage0
