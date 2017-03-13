#!/bin/sh
#set -x
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

failed() {
    echo "---------------------------------------------------------------------"
    echo "$1 $(pwd) failed"
    head -n 2000 "$build_dir/build.log"
    echo "---------------------------------------------------------------------"
    tail -n 2000 "$build_dir/build.log"
    echo "---------------------------------------------------------------------"
    exit 1
}

build_s1_pkg() {
    pkg="$1"
    build_dir="${buildparentdir}/build-${pkg}"
    root_dir="${build_dir}/root"


    # make sure it's not completed yet
    if [ -d "${outdir}/${pkg}" ] ; then
        echo "stage1: Skipping $pkg"
        return
    fi

    # figure out version
    phys_path="`cd ${packages}/${pkg}/*-\~\~stage1 && pwd -P`"
    control="$phys_path/control"
    ver="`basename $phys_path`"

    echo "stage1: Building $pkg $ver"

    # prepare build directory
    rm -rf "$build_dir"
    cp -r "$phys_path" "$build_dir"
    mkdir -p "$root_dir"

    pkgbuild -w "$build_dir" -o root -d "$distfiles" "$control" \
      >"$build_dir/build.log" 2>&1 || failed build

    cp -r "${build_dir}/root/packages/${pkg}" "$outdir/"
    cd "${buildparentdir}"
}

build_sdtools_pkg() {
    pkg="staticdistro-tools"
    build_dir="${buildparentdir}/build-${pkg}"
    root_dir="${build_dir}/root"

    # make sure it's not completed yet
    if [ -d "${outdir}/${pkg}" ] ; then
        echo "stage1: Skipping $pkg"
        return
    fi

    # figure out version
    phys_path="`cd ${toolsdir} && pwd -P`"
    control="$phys_path/control"
    ver="0.0.`date +'%Y%m%d%H%M'`~~stage1"

    echo "stage1: Building $pkg $ver"

    # prepare build directory
    rm -rf "$build_dir"
    cp -r "$phys_path" "$build_dir"
    mkdir -p "$root_dir"

    pkgbuild -w "$build_dir" -o "$root_dir" -d "$distfiles" -v "$ver" \
      "$control" >"$build_dir/build.log" 2>&1 || failed build

    cp -r "${build_dir}/root/packages/${pkg}" "$outdir/"
    cd "${buildparentdir}"
}

stage1_packages="binutils gcc musl sbase mksh make sed grep awk"
for p in $stage1_packages ; do
    build_s1_pkg $p
done
build_sdtools_pkg
