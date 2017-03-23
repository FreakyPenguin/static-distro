#!/bin/sh
#set -x
set -e

. ./common.sh

s2_pkgs="`pwd`/stage2_packages"
outdir="`pwd`/stage3_packages"
buildparentdir="`pwd`/stage3_build"

# create stage3_packages folder, copy stage2 packages
if [ ! -d "$outdir" ] ; then
    cp -ar $s2_pkgs $outdir
fi

# prepare directories
mkdir -p $buildparentdir

failed() {
    echo "---------------------------------------------------------------------"
    echo "$1 $build_dir failed"
    head -n 2000 "$build_dir/build.log"
    echo "---------------------------------------------------------------------"
    tail -n 2000 "$build_dir/build.log"
    echo "---------------------------------------------------------------------"
    exit 1
}

build_s3_pkg() {
    pkg="$1"
    build_dir="${buildparentdir}/build-${pkg}"
    out_dir="${build_dir}/root"

    # figure out version
    versions="`cd ${packages}/${pkg}/ && ls -1`"
    ver="`pkgvercmp -M -- $versions`"
    phys_path="${packages}/${pkg}/${ver}"
    control="$phys_path/control"
    #
    # make sure it's not completed yet
    if [ -d "${outdir}/${pkg}/${ver}~~stage3" ] ; then
        echo "stage3: Skipping $pkg/${ver}"
        return
    fi

    echo "stage3: Building $pkg $ver"

    # prepare build directory
    rm -rf "$build_dir"
    cp -r "$phys_path" "$build_dir"
    mkdir -p "$out_dir"

    # Build package
    pkgbuild -p "$outdir" -w "$build_dir" -d "$distfiles" -o root -V '~~stage3'\
      "$control" >>"$build_dir/build.log" 2>&1 || failed build

    for binpkg in `srccontrol -p "$control"`; do
        cp -ar "${build_dir}/root/packages/${binpkg}" "$outdir/"
    done
    cd "${buildparentdir}"
}

stage3_packages="musl musl-dynamic binutils m4 gmp mpfr mpc perl texinfo bison\
    flex patch gcc"
for p in $stage3_packages ; do
    build_s3_pkg $p
done
