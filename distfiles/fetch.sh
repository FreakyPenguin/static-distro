#!/bin/sh

set -e

GET="wget -nc"

if [ "$1" = "stage2" ] ; then
    ${GET} "https://ftp.gnu.org/gnu/tar/tar-1.29.tar.gz"
    ${GET} "https://ftp.gnu.org/gnu/gzip/gzip-1.8.tar.gz"
    ${GET} "http://www.bzip.org/1.0.6/bzip2-1.0.6.tar.gz"
    ${GET} "http://download.savannah.gnu.org/releases/lzip/lzip-1.18.tar.gz"
    ${GET} "http://sv0.famkaufmann.info/~staticdistro/distfiles/xz-5.2.2.tar.gz" || \
      ${GET} "http://tukaani.org/xz/xz-5.2.2.tar.gz"
    exit 0
fi

#stage 0:
${GET} "https://ftp.gnu.org/gnu/binutils/binutils-2.28.tar.gz"
${GET} "https://ftp.gnu.org/gnu/gcc/gcc-6.3.0/gcc-6.3.0.tar.gz"
${GET} "https://www.musl-libc.org/releases/musl-1.1.16.tar.gz"
${GET} "https://ftp.gnu.org/gnu/gmp/gmp-6.1.2.tar.lz"
${GET} "https://ftp.gnu.org/gnu/mpfr/mpfr-3.1.5.tar.gz"
${GET} "https://ftp.gnu.org/gnu/mpc/mpc-1.0.3.tar.gz"

if [ "$1" = "stage0" ] ; then
    exit 0
fi

if [ "$1" = "stage3" ] ; then
    # binutils m4 gmp mpfr mpc musl-dynamic perl texinfo bison flex patch gcc

    ${GET} "https://ftp.gnu.org/gnu/m4/m4-1.4.17.tar.gz"
    ${GET} "http://www.cpan.org/src/5.0/perl-5.24.1.tar.gz"
    ${GET} "https://ftp.gnu.org/gnu/texinfo/texinfo-6.1.tar.gz"
    ${GET} "https://ftp.gnu.org/gnu/bison/bison-3.0.4.tar.gz"
    ${GET} "https://github.com/westes/flex/releases/download/v2.6.1/flex-2.6.1.tar.gz"
    ${GET} "https://ftp.gnu.org/gnu/patch/patch-2.7.5.tar.gz"
    exit 0
fi

#stage 1:
${GET} "https://famkaufmann.info/~staticdistro/distfiles/sbase-0.0.20170323.tar.gz"
${GET} "http://sv0.famkaufmann.info/~staticdistro/distfiles/mksh-R54.tgz" || \
    ${GET} "https://www.mirbsd.org/MirOS/dist/mir/mksh/mksh-R54.tgz"
${GET} "https://ftp.gnu.org/gnu/make/make-4.2.1.tar.gz"
${GET} "https://ftp.gnu.org/gnu/sed/sed-4.4.tar.xz"
${GET} "https://ftp.gnu.org/gnu/grep/grep-3.0.tar.xz"
${GET} "https://ftp.gnu.org/gnu/gawk/gawk-4.1.4.tar.gz"

if [ "$1" = "stage1" ] ; then
    exit 0
fi

#stage 3:
${GET} "http://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.39.tar.gz"
${GET} "http://zlib.net/zlib-1.2.8.tar.gz"
${GET} "https://ftp.openbsd.org/pub/OpenBSD/LibreSSL/libressl-2.5.2.tar.gz"
${GET} "https://cdn.kernel.org/pub/linux/kernel/v4.x/linux-4.7.5.tar.xz"
${GET} "https://nginx.org/download/nginx-1.11.4.tar.gz"
${GET} "https://www.opensmtpd.org/archives/libasr-201602131606.tar.gz"
${GET} "http://sv0.famkaufmann.info/~staticdistro/distfiles/musl-fts-1.2.5.tar.gz"
${GET} "https://www.opensmtpd.org/archives/opensmtpd-6.0.0p1.tar.gz"
${GET} "https://github.com/libevent/libevent/releases/download/release-2.0.22-stable/libevent-2.0.22-stable.tar.gz"
${GET} "http://dovecot.org/releases/2.2/dovecot-2.2.26.0.tar.gz"
${GET} "http://skarnet.org/software/skalibs/skalibs-2.4.0.1.tar.gz"
${GET} "http://skarnet.org/software/execline/execline-2.2.0.0.tar.gz"
${GET} "http://skarnet.org/software/s6/s6-2.4.0.0.tar.gz"
${GET} "http://skarnet.org/software/s6-rc/s6-rc-0.1.0.0.tar.gz"
${GET} "https://famkaufmann.info/~staticdistro/distfiles/ubase-0.0.20160925.tar.gz"
${GET} "http://mirrors.sonic.net/pub/OpenBSD/OpenSSH/portable/openssh-7.3p1.tar.gz"
${GET} "http://dist.schmorp.de/libev/libev-4.24.tar.gz"
${GET} "https://curl.haxx.se/download/curl-7.53.1.tar.lzma"
${GET} "http://www.libarchive.org/downloads/libarchive-3.3.1.tar.gz"
${GET} "http://sv0.famkaufmann.info/~staticdistro/distfiles/expat-2.2.0.tar.bz2"
${GET} "http://sv0.famkaufmann.info/~staticdistro/distfiles/libuv-v1.9.1.tar.gz"
${GET} "http://www.cmake.org/files/v3.7/cmake-3.7.2.tar.gz"
${GET} "http://xmlsoft.org/sources/libxml2-2.9.4.tar.gz"
${GET} "https://kristaps.bsd.lv/acme-client/snapshots/acme-client-portable-0.1.16.tgz"
${GET} "http://dist.schmorp.de/libev/libev-4.24.tar.gz"
${GET} "https://hitch-tls.org/source/hitch-1.4.4.tar.gz"
${GET} "https://www.python.org/ftp/python/3.6.1/Python-3.6.1.tar.xz"
${GET} "http://sv0.famkaufmann.info/~staticdistro/distfiles/netbsd-curses-0.2.1.tar.gz"

# old versions
${GET} "https://ftp.gnu.org/gnu/binutils/binutils-2.26.1.tar.gz"
${GET} "https://ftp.gnu.org/gnu/gcc/gcc-5.3.0/gcc-5.3.0.tar.gz"
${GET} "https://www.musl-libc.org/releases/musl-1.1.14.tar.gz"
${GET} "https://famkaufmann.info/~staticdistro/distfiles/sbase-0.0.20161118.tar.gz"
${GET} "http://sv0.famkaufmann.info/~staticdistro/distfiles/mksh-R52c.tgz" || \
    ${GET} "https://www.mirbsd.org/MirOS/dist/mir/mksh/mksh-R52c.tgz"
${GET} "https://ftp.gnu.org/gnu/sed/sed-4.2.2.tar.gz"
${GET} "https://ftp.gnu.org/gnu/grep/grep-2.25.tar.xz"
${GET} "https://ftp.gnu.org/gnu/gawk/gawk-4.1.3.tar.gz"
${GET} "https://ftp.gnu.org/gnu/gmp/gmp-6.1.1.tar.lz"
${GET} "https://ftp.gnu.org/gnu/mpfr/mpfr-3.1.4.tar.gz"
${GET} "http://www.cpan.org/src/5.0/perl-5.24.0.tar.gz"
${GET} "http://ftp.openbsd.org/pub/OpenBSD/LibreSSL/libressl-2.5.0.tar.gz"
