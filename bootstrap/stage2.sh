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
    head -n 2000 build.log
    echo "---------------------------------------------------------------------"
    tail -n 2000 build.log
    echo "---------------------------------------------------------------------"
    exit 1
}

build_s2_pkg() {
    pkg="$1"
    build_dir="${buildparentdir}/build-${pkg}"

    # figure out version
    versions="`cd ${packages}/${pkg}/ && ls -1`"
    ver="`pkgvercmp -M -- $versions`"
    phys_path="${packages}/${pkg}/${ver}"
    control="$phys_path/control"
    #
    # make sure it's not completed yet
    if [ -d "${outdir}/${pkg}/${ver}" ] ; then
        echo "stage2: Skipping $pkg/${ver}"
        return
    fi

    echo "stage2: Building $pkg $ver"

    # prepare build directory
    rm -rf "$build_dir"
    cp -r "$phys_path" "$build_dir"
    cd "$build_dir"

    # get distfiles
    for f in `pkgcontrol -s $control` ; do
        cp "${distfiles}/${f}" .
    done

    # get build dependencies
    deps="-p mksh "
    for d in `pkgcontrol -b $control` ; do
        deps="$deps -p $d"
    done

    mkdir -p root

    export PKG_NAME="$pkg"
    export PKG_VERSION="$ver"
    export PKG_DIR="/packages/${pkg}/$ver"
    export PKG_INSTDIR="${build_dir}/root"

    echo "stage2:     Unpacking"
    ./unpack.sh >build.log 2>&1 || failed unpack

    echo "stage2:     Building"
    export PKG_INSTDIR="/work/root"
    withpkgs -d $outdir $deps -w . -- /bin/sh -c 'cd /work && ./build.sh' \
        >build.log 2>&1 || failed build

    # copy over control file
    cp "$control" "${build_dir}/root/packages/${pkg}/${ver}/control"

    cp -r "${build_dir}/root/packages/${pkg}" "$outdir/"
    cd "${buildparentdir}"
}

stage2_packages="tar binutils m4 gmp mpfr mpc musl-dynamic perl texinfo bison flex \
    binutils gcc"
for p in $stage2_packages ; do
    build_s2_pkg $p
done
