#!/bin/sh
set -e

arch_build="`gcc -dumpmachine`"
arch_target=x86_64-linux-musl

bs_prefix="`pwd`/bs_prefix"
workdir="`pwd`"
repodir="`pwd`/../packages/src"
packagedir="`pwd`/packages"

steps="binutils gcc_1 musl gcc_2 gmp mpfr mpc"
packages="binutils gcc musl sbase mksh make"
repo_binutils="$repodir/binutils-gdb"
repo_gcc="$repodir/gcc"
repo_musl="$repodir/musl"
repo_gmp="$repodir/gmp"
repo_mpfr="$repodir/mpfr"
repo_mpc="$repodir/mpc"
repo_sbase="$repodir/sbase"
repo_mksh="$repodir/mksh"
repo_make="$repodir/make"

build_bs_binutils() {
    git clone $repo_binutils src
    cd src
    git checkout static-bootstrap
    cd ..

    mkdir build
    cd build
    ../src/configure --prefix="$bs_prefix" --target=$arch_target \
        --with-sysroot="$bs_prefix" --disable-werror
    make -j8
    make install
}

build_bs_gcc_1() {
    git clone $repo_gcc src
    cd src
    git checkout static-bootstrap
    cd ..

    mkdir build
    cd build
    CFLAGS="-O0 -g0" CXXFLAGS="-O0 -g0" ../src/configure --prefix="$bs_prefix" \
        --target=$arch_target --with-sysroot="$bs_prefix" --with-newlib \
        --enable-languages=c --disable-libssp --disable-nls \
        --disable-libquadmath --disable-threads --disable-decimal-float \
        --disable-shared --disable-libmudflap --disable-libgomp \
        --disable-libatomic --disable-werror \
        --disable-multilib --with-multilib-list=
    make -j8
    make install

    # remove fixincludes
    rm -rf "$bs_prefix/lib/gcc/x86_64-linux-musl"/*/include-fixed/ \
        "$bs_prefix/lib/gcc/x86_64-linux-musl"/*/include/stddef.h
}

build_bs_musl() {
    git clone $repo_musl src
    cd src
    git checkout static-bootstrap
    ./configure --prefix="$bs_prefix" --build=$arch_build --host=$arch_target \
        --target=$arch_target -enable-optimize \
        CROSS_COMPILE="$arch_target-" CC="$arch_target-gcc"
    make -j8
    make install
}

build_bs_gcc_2() {
    git clone $repo_gcc src
    cd src
    git checkout static-bootstrap
    cd ..

    mkdir build
    cd build
    ../src/configure --prefix="$bs_prefix" \
        --target=$arch_target --with-sysroot="$bs_prefix" \
        --enable-languages=c,c++ --disable-libssp --disable-nls \
        --disable-libquadmath --disable-threads --disable-decimal-float \
        --disable-shared --disable-libmudflap --disable-libgomp \
        --disable-libatomic --disable-werror  --disable-libsanitizer \
        --disable-multilib --with-multilib-list=

    make -j8
    make install

    # remove fixincludes
    rm -rf "$bs_prefix/lib/gcc/x86_64-linux-musl"/*/include-fixed/ \
        "$bs_prefix/lib/gcc/x86_64-linux-musl"/*/include/stddef.h
}

build_bs_gmp() {
    git clone $repo_gmp src
    cd src
    git checkout static-bootstrap
    cd ..

    mkdir build
    cd build
    ../src/configure --prefix="$bs_prefix" --build=$arch_build \
        --target=$arch_target --host=$arch_target --disable-werror
    make -j8
    make install
}

build_bs_mpfr() {
    git clone $repo_mpfr src
    cd src
    git checkout static-bootstrap
    cd ..

    mkdir build
    cd build
    ../src/configure --prefix="$bs_prefix" --build=$arch_build \
        --target=$arch_target --host=$arch_target --disable-werror
    make -j8
    make install
}

build_bs_mpc() {
    git clone $repo_mpc src
    cd src
    git checkout static-bootstrap
    autoreconf -i
    cd ..

    mkdir build
    cd build
    # TODO: figure out why we need to override CC
    ../src/configure --prefix="$bs_prefix" --build=$arch_build \
        --target=$arch_target --host=$arch_target --disable-werror \
        CC=$arch_target-gcc
    make -j8
    make install
}

build_pkg_binutils() {
    git clone $repo_binutils src
    cd src
    git checkout static-bootstrap
    cd ..

    mkdir build
    cd build
    ../src/configure --prefix="$prefix" \
        --oldincludedir=$prefix/include --infodir=$prefix/info \
        --localedir=$prefix/locale --mandir=$prefix/man --docdir=$prefix/doc \
        --build=$arch_build --target=$arch_target --host=$arch_target \
        --disable-werror --disable-nls --disable-gdb --disable-libdecnumber \
        --disable-readline --disable-sim
    make configure-host
    make -j8 LDFLAGS=-all-static
    make install DESTDIR=$rootdir

    # remove libraries and headers
    rm -rf $rootdir/$prefix/lib $rootdir/$prefix/include
}

build_pkg_gcc() {
    git clone $repo_gcc src
    cd src
    git checkout static-bootstrap
    cd ..

    mkdir build
    cd build
    ../src/configure --prefix="$prefix" \
        --oldincludedir=$prefix/include --infodir=$prefix/info \
        --localedir=$prefix/locale --mandir=$prefix/man --docdir=$prefix/doc \
        --build=$arch_build --target=$arch_target --host=$arch_target \
        --enable-languages=c,c++ --disable-nls --disable-libmudflap \
        --disable-libsanitizer --disable-werror \
        --disable-shared --disable-host-shared --enable-static \
        LDFLAGS=-static
    make -j8
    make install DESTDIR=$rootdir
}

build_pkg_musl() {
    git clone $repo_musl src

    cd src
    git checkout static-bootstrap
    ./configure --prefix="$prefix" --build=$arch_build --host=$arch_target \
        --target=$arch_target -enable-optimize --syslibdir=$prefix/lib \
        CROSS_COMPILE="$arch_target-" CC="$arch_target-gcc"
    make -j8
    make install DESTDIR=$rootdir
}

build_pkg_sbase() {
    git clone $repo_sbase src

    cd src
    git checkout static-bootstrap
    make -j8
    make install DESTDIR=$rootdir
    rm $rootdir/$prefix/bin/strings
}

build_pkg_mksh() {
    git clone $repo_mksh src

    cd src
    git checkout static-bootstrap
    CC=$arch_target-gcc LDSTATIC=-static HAVE_STRLCPY=0 sh Build.sh -r -c lto
    mkdir -p $rootdir/packages/mksh/bootstrap/bin
    mkdir -p $rootdir/packages/mksh/bootstrap/doc/examples
    mkdir -p $rootdir/packages/mksh/bootstrap/man/man1
    cp mksh $rootdir/packages/mksh/bootstrap/bin/
    ln -s mksh $rootdir/packages/mksh/bootstrap/bin/sh
    cp dot.mkshrc $rootdir/packages/mksh/bootstrap/doc/examples/
    cp mksh.1 $rootdir/packages/mksh/bootstrap/man/man1/
}

build_pkg_make() {
    git clone $repo_make src
    cd src
    git checkout static-bootstrap
    autoreconf -i
    cd ..

    mkdir build
    cd build
    ../src/configure --prefix="$prefix" \
        --oldincludedir=$prefix/include --infodir=$prefix/info \
        --localedir=$prefix/locale --mandir=$prefix/man --docdir=$prefix/doc \
        --build=$arch_build --target=$arch_target --host=$arch_target \
        --disable-shared LDFLAGS=-static
    make update
    make -j8
    make install DESTDIR=$rootdir
}

# prepare prefix dir
mkdir -p $bs_prefix
# make sure prefix/usr/{lib,include} are symlinked to the dirs in prefix
mkdir -p $bs_prefix/usr
if [ ! -L "$bs_prefix/usr/lib" ] ; then
    ln -s $bs_prefix/lib $bs_prefix/usr/lib
fi
if [ ! -L "$bs_prefix/usr/include" ] ; then
    ln -s $bs_prefix/include $bs_prefix/usr/include
fi

mkdir -p $packagedir



for step in $steps; do
    dir=$workdir/build-bs_${step}
    if [ -f $dir/.done ] ; then
        echo "Skipping BS $step"
        continue
    fi

    echo "Building BS $step"

    # create work dir
    rm -rf $dir
    mkdir $dir
    cd $dir

    "build_bs_$step" >build.log 2>&1

    touch $dir/.done
    cd $workdir
done


for pkg in $packages; do
    dir=$workdir/build-pkt_${pkg}
    prefix="/packages/$pkg/bootstrap"
    rootdir="$dir/root"

    if [ -f $dir/.done ] ; then
        echo "Skipping package $pkg"
        continue
    fi

    echo "Building packet $pkg"

    # create work dir
    rm -rf $dir
    mkdir $dir
    cd $dir
    mkdir -p $rootdir

    "build_pkg_$pkg" >build.log 2>&1

    # archive package
    cd $rootdir
    rm -f $packagedir/${pkg}-bootstrap.tar.bz2
    tar cjf $packagedir/${pkg}-bootstrap.tar.bz2 packages/${pkg}/bootstrap

    touch $dir/.done
    cd $workdir
done
