#!/bin/sh
#set -x
set -e

. ./common.sh

s1_pkgs="`pwd`/stage1_packages"
outdir="`pwd`/stage2_packages"
buildparentdir="`pwd`/stage2_build"

export PATH="$toolsdir:$PATH"

# create stage2_packages folder, copy stage1 packages
if [ ! -d "$outdir" ] ; then
    cp -r $s1_pkgs $outdir
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

build_s2_pkg() {
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
    if [ -d "${outdir}/${pkg}/${ver}~~stage2" ] ; then
        echo "stage2: Skipping $pkg/${ver}"
        return
    fi

    echo "stage2: Building $pkg $ver"

    # prepare build directory
    rm -rf "$build_dir"
    cp -r "$phys_path" "$build_dir"
    mkdir -p "$out_dir"

    # Unpack package
    pkgbuild -B -C -w "$build_dir" -d "$distfiles" -V '-~~stage2' "$control" \
      >"$build_dir/build.log" 2>&1 || failed unpack

    # Build package
    pkgbuild -U -p "$outdir" -w "$build_dir" -o root -V '-~~stage2' "$control" \
      >>"$build_dir/build.log" 2>&1 || failed build

    for binpkg in `srccontrol -p "$control"`; do
        cp -ar "${build_dir}/root/packages/${binpkg}" "$outdir/"
    done
    cd "${buildparentdir}"
}

stage2_packages="tar gzip bzip2 lzip xz"
for p in $stage2_packages ; do
    build_s2_pkg $p
done
