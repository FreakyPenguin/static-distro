CFLAGS= -O2 -Wall -Wextra

all: withenv pkgcontrol pkgsolve
clean:
	rm -f *.o withenv pkgcontrol pkgsolve

withenv: withenv.o
pkgcontrol: pkgcontrol.o control.o
pkgsolve: pkgsolve.o solve.o control.o

.PHONY: all clean
