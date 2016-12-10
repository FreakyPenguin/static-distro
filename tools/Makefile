CFLAGS= -O2 -Wall -Wextra

EXECS= newns withenv withpkgs pkgcontrol pkgsolve pkgvercmp

all: $(EXECS)
clean:
	rm -f *.o $(EXECS)


newns: newns.o
withenv: withenv.o
withpkgs: withpkgs.o solve.o control.o version.o repo.o
pkgcontrol: pkgcontrol.o control.o
pkgsolve: pkgsolve.o solve.o control.o version.o repo.o
pkgvercmp: pkgvercmp.o version.o

.PHONY: all clean
