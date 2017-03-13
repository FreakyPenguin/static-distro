#!/bin/sh
#set -x
set -e

s1_pkgs="`pwd`/stage1_packages"
distfiles="`pwd`/../distfiles"
packages="`pwd`/../packages"
toolsdir="`pwd`/../tools"
outdir="`pwd`/stage2_packages"
buildparentdir="`pwd`/stage2_build"

export ARCH=x86_64-linux-musl
export ARCH_BUILD=$ARCH
export ARCH_TARGET=$ARCH
export PATH="$toolsdir:$PATH"

# create stage2_packages folder, copy stage1 packages
if [ ! -d "$outdir" ] ; then
    cp -r $s1_pkgs $outdir
fi

# prepare directories
mkdir -p $buildparentdir

failed() {
    echo "---------------------------------------------------------------------"
    echo "$1 $(pwd) failed"
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

    # get build dependencies
    deps="-p staticdistro-tools"
    for d in `srccontrol -b $control` ; do
        deps="$deps -p $d"
    done

    # Unpack package
    pkgbuild -B -C -w "$build_dir" -d "$distfiles" "$control" -V '~~stage2' \
      >"$build_dir/build.log" 2>&1 || failed unpack

    # Build package
    newns withpkgs -d "$outdir" $deps -w "$build_dir" -- \
      pkgbuild -U -w "/work" -o "/work/root" -V '~~stage2' "/work/control" \
      >>"$build_dir/build.log" 2>&1 || failed build

    cp -r "${build_dir}/root/packages/${pkg}" "$outdir/"
    cd "${buildparentdir}"
}

stage2_packages="tar binutils m4 gmp mpfr mpc musl-dynamic perl texinfo bison flex \
    gcc"
for p in $stage2_packages ; do
    build_s2_pkg $p
done
