default:
	gcc herbe.c -lX11 -lXft -I/usr/include/freetype2 -o herbe

install: default
	cp herbe /usr/local/bin

clean:
	rm -f herbe
