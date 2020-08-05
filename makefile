default:
	gcc herbe.c -Wall -Wextra -Werror -pedantic -lX11 -lXft -I/usr/include/freetype2 -lm -o herbe

install: default
	cp herbe /usr/local/bin

uninstall:
	rm /usr/local/bin/herbe

clean:
	rm -f herbe
