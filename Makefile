CC ?= cc
PREFIX ?= /usr/local

HERBE_CFLAGS = -Wall -Wextra -pedantic -lX11 -lXft -I/usr/include/freetype2 -pthread

all: herbe

config.h: config.def.h
	cp -f config.def.h config.h

herbe: herbe.c config.h
	$(CC) herbe.c $(CFLAGS) $(HERBE_CFLAGS) $(LDFLAGS) -o herbe

install: herbe
	install -d ${DESTDIR}${PREFIX}/bin
	install -m 755 herbe ${DESTDIR}${PREFIX}/bin

uninstall:
	rm -f ${DESTDIR}${PREFIX}/bin/herbe

clean:
	rm -f herbe

.PHONY: all install uninstall clean
