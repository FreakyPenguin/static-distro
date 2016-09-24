CFLAGS= -O2 -Wall -Wextra

all: withenv pkgcontrol pkgsolve pkgvercmp
clean:
	rm -f *.o withenv pkgcontrol pkgsolve pkgvercmp

withenv: withenv.o
pkgcontrol: pkgcontrol.o control.o
pkgsolve: pkgsolve.o solve.o control.o version.o
pkgvercmp: pkgvercmp.o version.o

.PHONY: all clean
