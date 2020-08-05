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

	char *body = argv[1];

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

	int screen_width = DisplayWidth(display, screen);
	int screen_height = DisplayHeight(display, screen);

	XftColor color;

	XSetWindowAttributes attributes;
	attributes.override_redirect = True;
	XftColorAllocName(display, visual, colormap, background_color, &color);
	attributes.background_pixel = color.pixel;
	XftColorAllocName(display, visual, colormap, border_color, &color);
	attributes.border_pixel = color.pixel;

	XftFont *font = XftFontOpenName(display, screen, font_pattern);

	int max_text_width = width - 2 * padding;
	int eols_size = 5;
	int *eols = malloc(eols_size * sizeof(int));
	eols[0] = 0;
	int remainder = strlen(body);
	int num_of_lines = 1;

	while (1)
	{
		XGlyphInfo info;
		info.width = 0;
		int eol = max_text_width / font->max_advance_width;
		while (info.width < max_text_width)
		{
			eol++;
			XftTextExtentsUtf8(display, font, (FcChar8 *)body + eols[num_of_lines - 1], eol, &info);
		}

		--eol;

		if (eol >= remainder)
		{
			if (eols_size < num_of_lines + 1)
			{
				eols_size += 5;
				eols = realloc(eols, eols_size * sizeof(int));
			}
			eols[num_of_lines] = eols[num_of_lines - 1] + remainder;
			num_of_lines++;
			break;
		}

		int temp = eol;

		while (body[eols[num_of_lines - 1] + eol] != ' ' && eol)
			--eol;

		if (eol == 0)
			eol = temp;
		else
			eol++;

		remainder -= eol;
		if (eols_size < num_of_lines + 1)
		{
			eols_size += 5;
			eols = realloc(eols, eols_size * sizeof(int));
		}
		eols[num_of_lines] = eols[num_of_lines - 1] + eol;
		num_of_lines++;
	}

	unsigned int x = pos_x;
	unsigned int y = pos_y;
	unsigned int text_height = font->ascent - font->descent;
	unsigned int height = (num_of_lines - 2) * line_spacing + (num_of_lines - 1) * text_height + 2 * padding;

	if (corner == TOP_RIGHT || corner == BOTTOM_RIGHT)
		x = screen_width - width - border_size * 2 - pos_x;

	if (corner == BOTTOM_LEFT || corner == BOTTOM_RIGHT)
		y = screen_height - height - border_size * 2 - pos_y;

	window = XCreateWindow(display, RootWindow(display, screen), x, y, width, height, border_size, DefaultDepth(display, screen), CopyFromParent, visual,
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
			for (int i = 1; i < num_of_lines; i++)
				XftDrawStringUtf8(draw, &color, font, padding, line_spacing * (i - 1) + text_height * i + padding, (FcChar8 *)body + eols[i - 1], eols[i] - eols[i - 1]);
		}
		if (event.type == ButtonPress)
			break;
	}

	free(eols);
	XftDrawDestroy(draw);
	XftColorFree(display, visual, colormap, &color);
	XftFontClose(display, font);

	XCloseDisplay(display);
	return EXIT_SUCCESS;
}
