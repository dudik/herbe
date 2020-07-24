#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "config.h"

int main(int argc, char *argv[])
{
	Display *display = XOpenDisplay(NULL);
	XEvent event;

	if (display == NULL)
	{
		fprintf(stderr, "Cannot open display\n");
		exit(EXIT_FAILURE);
	}

	int screen = DefaultScreen(display);

	int window_width = DisplayWidth(display, screen);
	int widnow_height = DisplayHeight(display, screen);

	Window root = RootWindow(display, screen);
	XSetWindowAttributes attributes;
	attributes.override_redirect = True;
	attributes.background_pixel = background_color;
	attributes.border_pixel = border_color;

	XftColor color;
	char *status = "Ahoj volam sa samko netahaj mi stolicku lebo ta ujebem ty hovafoooooo";
	XftFont *font = XftFontOpenName(display, screen, font_style);

	Window window = XCreateWindow(
		display, root, pos_x,
		pos_y, width, font->ascent + 10 + border_size, border_size,
		DefaultDepth(display, screen), CopyFromParent,
		DefaultVisual(display, screen),
		CWOverrideRedirect | CWBackPixel | CWBorderPixel, &attributes);

	XftDraw *draw = XftDrawCreate(display, window, DefaultVisual(display, screen), DefaultColormap(display, screen));
	XftColorAllocName(display, DefaultVisual(display, screen), DefaultColormap(display, screen), "#000000", &color);

	XMapWindow(display, window);

	XftDrawString8(draw, &color, font, 5, font->ascent + 5, (XftChar8 *)argv[1], strlen(argv[1]));

	XNextEvent(display, &event);

	sleep(duration);
}
