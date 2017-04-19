# static-distro

This is an experiment: building a linux distribution of statically linked
packages. The main focus is building lightweight containers.

Packages are installed under `/packages/$NAME/$VERSION`. The core utility
at this point is `withpkgs` which runs a command in an isolated chroot
containing a subset of packages. For example:

    $ withpkgs -d bootstrap/stage2_packages -p mksh sbase -p musl -p gcc \
        -w my_working_directory /bin/sh

This runs a container with a shell, basic utilities and gcc, mounts the
directory `my_working_directory` under `/work` in the container and then runs
`/bin/sh` in the container. `withpkgs` does dependency resolution, so the
container will also include binutils for instance.

## Bootstrap
static-distro can be bootstrapped from any Linux distribution. For Ubuntu/Debian
the following packages need to be installed for doing a full bootstrap starting
with stage 0:

    build-essential libgmp-dev libmpc-dev libmpfr-dev lzip m4

Bootstrapping is divided into 3 stages:
 * `stage0`: Build a cross compiler and the `musl` libc (runs on host
        distribution).
 * `stage1`: Use cross compiler to build transitional packages needed to *build*
        stage2+ packages fully inside a static-distro chroot without depending
        on the host system.
 * `stage2`: Uses only stage1 packages necessary to build packages necessary for
        *unpacking* and building further packages fully inside a static-distro
        chroot. stage2 packages are still unpacked on the host.
 * `stage3`: Use only stage1 and stage2 packages to *unpack and build*
       additional packages fully inside an insolated chroot.

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

The result of this is the repository of packages in `bootstrap/stage2_packages`.



## Tools
### Main tools
 * `withpkgs`: running a command in an environment including particular
   packages, with (or without) version constraints. (uses `withenv`)
 * `pkgbuild`: build a source package. (uses `withpkgs`)
 * `withenv`: mechanism for running a command in an environment consisting of
   particular package versions.

### Scripting tools
 * `pkgcontrol`: extract information from package control files.
 * `pkgsolve`: solve package constraints and yield a list of package versions.
 * `pkgvercmp`: compare package version numbers
