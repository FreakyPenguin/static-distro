rootdir="`pwd`/.."
distfiles="$rootdir/distfiles"
packages="$rootdir/packages"
toolsdir="$rootdir/tools"

export ARCH=x86_64-linux-musl
export ARCH_BUILD=$ARCH
export ARCH_TARGET=$ARCH

if [ -z "$MAKE_JOBS" ] ; then
    if which nproc >/dev/null 2>/dev/null ; then
        export MAKE_JOBS="-j `nproc`"
    else
        >&2 echo "nproc not available running with 1 make job"
    fi
fi
