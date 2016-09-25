CFLAGS= -O2 -Wall -Wextra

all: withenv withpkgs pkgcontrol pkgsolve pkgvercmp
clean:
	rm -f *.o withenv withpkgs pkgcontrol pkgsolve pkgvercmp

withenv: withenv.o
withpkgs: withpkgs.o solve.o control.o version.o repo.o
pkgcontrol: pkgcontrol.o control.o
pkgsolve: pkgsolve.o solve.o control.o version.o repo.o
pkgvercmp: pkgvercmp.o version.o

.PHONY: all clean
