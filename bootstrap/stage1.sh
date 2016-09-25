#!/bin/sh
set -x
set -e

s0_prefix="`pwd`/stage0_prefix"
distfiles="`pwd`/../distfiles"
packages="`pwd`/../packages"
toolsdir="`pwd`/../tools"
outdir="`pwd`/stage1_packages"
buildparentdir="`pwd`/stage1_build"

export ARCH_BUILD="`gcc -dumpmachine`"
export ARCH_TARGET=x86_64-linux-musl
export PATH="$s0_prefix/bin:$toolsdir:$PATH"

# prepare directories
mkdir -p $outdir $buildparentdir

build_s1_pkg() {
    pkg="$1"
    build_dir="${buildparentdir}/build-${pkg}"

    # make sure it's not completed yet
    if [ -d "${outdir}/${pkg}" ] ; then
        echo "stage1: Skipping $pkg"
        return
    fi

    # figure out version
    phys_path="`cd ${packages}/${pkg}/*-\~\~stage1 && pwd -P`"
    control="$phys_path/control"
    ver="`basename $phys_path`"

    echo "stage1: Building $pkg"

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
    export PKG_DIR="/packages/${pkg}/$ver"
    export PKG_INSTDIR="${build_dir}/root"

    mkdir -p ${PKG_INSTDIR}

    # unpack
    ./unpack.sh >build.log 2>&1

    # build pass
    ./build.sh >>build.log 2>&1

    # copy over control file
    cp "$control" "${build_dir}/root/packages/${pkg}/${ver}/control"

    cp -r "${build_dir}/root/packages/${pkg}" "$outdir/"
    cd "${buildparentdir}"
}

stage1_packages="binutils gcc musl sbase mksh make sed grep awk"
for p in $stage1_packages ; do
    echo $p
    build_s1_pkg $p
done
