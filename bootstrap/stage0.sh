#!/bin/sh
set -x
set -e

s0_prefix="`pwd`/stage0_prefix"
distfiles="`pwd`/../distfiles"
packages="`pwd`/../packages"
toolsdir="`pwd`/../tools"
buildparentdir="`pwd`/stage0_build"

export ARCH_BUILD="`gcc -dumpmachine`"
export ARCH_TARGET=x86_64-linux-musl
export PATH="$s0_prefix/bin:$toolsdir:$PATH"

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

failed() {
    echo "---------------------------------------------------------------------"
    echo "$1 $(pwd) failed"
    head -n 2000 build.log
    echo "---------------------------------------------------------------------"
    tail -n 2000 build.log
    echo "---------------------------------------------------------------------"
    exit 1
}

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
    phys_path="`cd ${packages}/${pkg}/*-\~\~${pass} && pwd -P`"
    control="$phys_path/control"
    ver="`basename $phys_path`"

    echo "stage0: Building $pkg pass $pass"

    # prepare build directory
    rm -rf "$build_dir"
    cp -r "$phys_path" "$build_dir"
    cd "$build_dir"

    # get distfiles
    for f in `pkgcontrol -s $control` ; do
        cp "${distfiles}/${f}" .
    done

    export PKG_NAME="$pkg"
    export PKG_VERSION="$ver"
    export PKG_DIR="$s0_prefix"
    export PKG_INSTDIR="/"

    # unpack
    ./unpack.sh >build.log 2>&1 || failed unpacked

    # build pass
    ./build.sh >>build.log 2>&1 || failed build

    touch "${build_dir}/.done"
    cd "${buildparentdir}"
}

build_s0_pkg binutils stage0
build_s0_pkg gcc stage0.1
build_s0_pkg musl stage0
build_s0_pkg gcc stage0.2
build_s0_pkg gmp stage0
build_s0_pkg mpfr stage0
build_s0_pkg mpc stage0
