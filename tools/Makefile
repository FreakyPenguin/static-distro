CFLAGS= -O2 -Wall -Wextra

PREFIX= /usr/local
DESTDIR= /
BINSUBDIR= bin

EXECS= newns withenv withpkgs pkgcontrol pkgsolve pkgvercmp gencontrol \
       srccontrol pkgbuild

all: $(EXECS)
clean:
	rm -f *.o $(EXECS)
install: $(EXECS)
	mkdir -p $(DESTDIR)/$(PREFIX)/$(BINSUBDIR)
	cp $(EXECS) $(DESTDIR)/$(PREFIX)/$(BINSUBDIR)/

newns: newns.o
withenv: withenv.o
withpkgs: withpkgs.o solve.o control.o controlparse.o version.o repo.o
pkgbuild: pkgbuild.o control.o controlparse.o
pkgcontrol: pkgcontrol.o control.o controlparse.o
pkgsolve: pkgsolve.o solve.o control.o controlparse.o version.o repo.o
pkgvercmp: pkgvercmp.o version.o
gencontrol: gencontrol.o control.o controlparse.o
srccontrol: srccontrol.o control.o controlparse.o

.PHONY: all clean install
