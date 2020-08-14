default:
	egcc herbe.c -Wall -Wextra -pedantic -I/usr/X11R6/include -L/usr/X11R6/lib -lX11 -lXft -I/usr/X11R6/include/freetype2 -lm -o herbe

install: default
	cp herbe /usr/local/bin

uninstall:
	rm /usr/local/bin/herbe

clean:
	rm -f herbe
