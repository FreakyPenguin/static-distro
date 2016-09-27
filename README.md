# static-distro

## Bootstrap
static-distro can be bootstrapped from any Linux distribution. For Ubuntu/Debian
the following packages need to be installed:

    build-essential libgmp-dev libmpc-dev libmpfr-dev lzip

Bootstrapping is divided into 3 stages:
 * `stage0`: Build a cross compiler and the `musl` libc (runs on host
        distribution).
 * `stage1`: Use cross compiler to transitional packages needed to build
        stage2 packages inside a static-distro chroot.
 * `stage2`: Use only stage1 packages to build gcc and it's dependencies within
        an insolated chroot.

### Building

    # Build static-distro tools
    $ make -C tools

    # withenv binary needs to be setuid root to be able to chroot
    # (this step requires sudo)
    $ make -C tools setuid

    # Fetch distfiles (source archives for all the packages)
    $ cd distfiles && ./fetch.sh && cd ..

    # Actually run bootstrapping steps (this will take a while)
    $ cd bootstrap && ./stage0.sh && ./stage1.sh && ./stage1.sh && cd ..

The result of this is the repository of packets in bootstrap/stage2_packages



## Tools
### Main tools
 * `withenv`: mechanism for running a command in an environment consisting of
   particular packet versions.
 * `withpkgs`: running a command in an environment including particular packages,
   with (or without) version constraints. (uses `withenv`)
 * `pkgbuild`: build a source package. (uses `withpkgs`)

### Scripting tools
 * `pkgcontrol`: extract information from packet control files.
 * `pkgsolve`: solve packet constraints and yield a list of packet versions.
 * `pkgvercmp`: compare packet version numbers
