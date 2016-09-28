#!/bin/sh

GET="wget -nc"

#stage 0:
${GET} "https://ftp.gnu.org/gnu/binutils/binutils-2.26.1.tar.gz"
${GET} "https://ftp.gnu.org/gnu/gcc/gcc-5.3.0/gcc-5.3.0.tar.gz"
${GET} "https://www.musl-libc.org/releases/musl-1.1.14.tar.gz"
${GET} "https://ftp.gnu.org/gnu/gmp/gmp-6.1.1.tar.lz"
${GET} "https://ftp.gnu.org/gnu/mpfr/mpfr-3.1.4.tar.gz"
${GET} "https://ftp.gnu.org/gnu/mpc/mpc-1.0.3.tar.gz"

##stage 1:
##${GET} "sbase-0.0.20160625.tar.gz"
#${GET} "https://www.mirbsd.org/MirOS/dist/mir/mksh/mksh-R52c.tar.gz"
#${GET} "https://ftp.gnu.org/gnu/make/make-4.2.1.tar.gz"
#${GET} "https://ftp.gnu.org/gnu/sed/sed-4.2.2.tar.gz"
#${GET} "https://ftp.gnu.org/gnu/grep/grep-2.25.tar.xz"
#${GET} "https://ftp.gnu.org/gnu/gawk/gawk-4.1.3.tar.gz"
#
##stage 2:
#${GET} "https://ftp.gnu.org/gnu/m4/m4-1.4.17.tar.gz"
#${GET} "http://www.cpan.org/src/5.0/perl-5.24.0.tar.gz"
#${GET} "https://ftp.gnu.org/gnu/texinfo/texinfo-6.1.tar.gz"
#${GET} "https://ftp.gnu.org/gnu/bison/bison-3.0.4.tar.gz"
#${GET} "https://github.com/westes/flex/releases/download/v2.6.1/flex-2.6.1.tar.gz"
#${GET} "http://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.39.tar.gz"
#${GET} "http://zlib.net/zlib-1.2.8.tar.gz"
#${GET} "http://ftp.openbsd.org/pub/OpenBSD/LibreSSL/libressl-2.5.0.tar.gz"
#${GET} "https://cdn.kernel.org/pub/linux/kernel/v4.x/linux-4.7.5.tar.xz"
#${GET} "https://nginx.org/download/nginx-1.11.4.tar.gz"
#${GET} "https://www.opensmtpd.org/archives/opensmtpd-6.0.0p1.tar.gz"
#${GET} "https://github.com/libevent/libevent/releases/download/release-2.0.22-stable/libevent-2.0.22-stable.tar.gz"
#
##beyond:
#${GET} "https://ftp.gnu.org/gnu/gzip/gzip-1.8.tar.gz"

