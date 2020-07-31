#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "config.h"

Display *display;
Window window;

void expire()
{
	XEvent event;
	event.type = ButtonPress;
	XSendEvent(display, window, 0, 0, &event);
	XFlush(display);
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		fprintf(stderr, "Usage: herbe message\n");
		exit(EXIT_FAILURE);
	}

	signal(SIGALRM, expire);
	alarm(duration);

	display = XOpenDisplay(NULL);

	if (display == NULL)
	{
		fprintf(stderr, "Cannot open display\n");
		exit(EXIT_FAILURE);
	}

	int screen = DefaultScreen(display);
	Visual *visual = DefaultVisual(display, screen);
	Colormap colormap = DefaultColormap(display, screen);

	int window_width = DisplayWidth(display, screen);
	int window_height = DisplayHeight(display, screen);

	XftColor color;

	XSetWindowAttributes attributes;
	attributes.override_redirect = True;
	XftColorAllocName(display, visual, colormap, background_color, &color);
	attributes.background_pixel = color.pixel;
	XftColorAllocName(display, visual, colormap, border_color, &color);
	attributes.border_pixel = color.pixel;

	XftFont *font = XftFontOpenName(display, screen, font_pattern);

	unsigned int x = pos_x;
	unsigned int y = pos_y;
	unsigned int height = font->ascent - font->descent + padding * 2;

	switch (corner)
	{
	case BOTTOM_RIGHT:
		y = window_height - height - border_size * 2 - pos_y;
	case TOP_RIGHT:
		x = window_width - width - border_size * 2 - pos_x;
		break;
	case BOTTOM_LEFT:
		y = window_height - height - border_size * 2 - pos_y;
	}

	window = XCreateWindow(
		display, RootWindow(display, screen), x,
		y, width, height, border_size,
		DefaultDepth(display, screen), CopyFromParent,
		visual,
		CWOverrideRedirect | CWBackPixel | CWBorderPixel, &attributes);

	XftDraw *draw = XftDrawCreate(display, window, visual, colormap);
	XftColorAllocName(display, visual, colormap, font_color, &color);

	XSelectInput(display, window, ExposureMask | ButtonPress);

	XMapWindow(display, window);

	XEvent event;

	while (1)
	{
		XNextEvent(display, &event);

		if (event.type == Expose)
		{
			XClearWindow(display, window);
			XftDrawStringUtf8(draw, &color, font, padding, height - padding, (XftChar8 *)argv[1], strlen(argv[1]));
		}
		if (event.type == ButtonPress)
			break;
	}

	XftDrawDestroy(draw);
	XftColorFree(display, visual, colormap, &color);
	XftFontClose(display, font);

	XCloseDisplay(display);
	return EXIT_SUCCESS;
}
