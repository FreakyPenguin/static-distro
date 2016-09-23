CFLAGS= -O2 -Wall -Wextra

all: withenv pkgcontrol
clean:
	rm -f *.o withenv pkgcontrol

withenv: withenv.o
pkgcontrol: pkgcontrol.o control.o

.PHONY: all clean
