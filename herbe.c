#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "config.h"

Display *display;
Window window;

static void die(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(EXIT_FAILURE);
}

int get_max_len(char *body, XftFont *font, int max_text_width)
{
	int body_len = strlen(body);
	XGlyphInfo info;
	XftTextExtentsUtf8(display, font, (FcChar8 *)body, body_len, &info);

	if (info.width < max_text_width)
		return body_len;

	int eol = max_text_width / font->max_advance_width;
	info.width = 0;

	while (info.width < max_text_width)
	{
		eol++;
		XftTextExtentsUtf8(display, font, (FcChar8 *)body, eol, &info);
	}

	eol--;

	int temp = eol;

	while (body[eol] != ' ' && eol)
		--eol;

	if (eol == 0)
		return temp;
	else
		return ++eol;
}

void expire()
{
	XEvent event;
	event.type = ButtonPress;
	XSendEvent(display, window, 0, 0, &event);
	XFlush(display);
}

int main(int argc, char *argv[])
{
	if (argc == 1)
		die("Usage: %s body", argv[0]);

	signal(SIGALRM, expire);
	alarm(duration);

	display = XOpenDisplay(0);

	if (display == 0)
		die("Cannot open display");

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

	int num_of_lines = 0;
	int max_text_width = width - 2 * padding;
	int words_size = 5;
	char **words = malloc(words_size * sizeof(char *));
	if (!words)
		die("malloc failed");

	for (int i = 1; i < argc; i++)
	{
		char *body = argv[i];

		for (unsigned int eol = get_max_len(body, font, max_text_width); eol <= strlen(body) && eol; body += eol, num_of_lines++, eol = get_max_len(body, font, max_text_width))
		{
			if (words_size <= num_of_lines)
			{
				words = realloc(words, (words_size += 5) * sizeof(char *));
				if (!words)
					die("malloc failed");
			}
			words[num_of_lines] = malloc((eol + 1) * sizeof(char));
			if (!words[num_of_lines])
				die("malloc failed");
			strncpy(words[num_of_lines], body, eol);
			words[num_of_lines][eol] = '\0';
		}
	}

	unsigned int x = pos_x;
	unsigned int y = pos_y;
	unsigned int text_height = font->ascent - font->descent;
	unsigned int height = (num_of_lines - 1) * line_spacing + num_of_lines * text_height + 2 * padding;

	if (corner == TOP_RIGHT || corner == BOTTOM_RIGHT)
		x = screen_width - width - border_size * 2 - pos_x;

	if (corner == BOTTOM_LEFT || corner == BOTTOM_RIGHT)
		y = screen_height - height - border_size * 2 - pos_y;

	window = XCreateWindow(display, RootWindow(display, screen), x, y, width, height, border_size, DefaultDepth(display, screen), CopyFromParent, visual,
						   CWOverrideRedirect | CWBackPixel | CWBorderPixel, &attributes);

	XftDraw *draw = XftDrawCreate(display, window, visual, colormap);
	XftColorAllocName(display, visual, colormap, font_color, &color);

	#ifdef SHORTCUT_KEY
		XGrabKey(display, XKeysymToKeycode(display, SHORTCUT_KEY), SHORTCUT_MOD, DefaultRootWindow(display), True, GrabModeAsync, GrabModeAsync);
		XGrabKey(display, XKeysymToKeycode(display, SHORTCUT_KEY), SHORTCUT_MOD | LockMask, DefaultRootWindow(display), True, GrabModeAsync, GrabModeAsync);
		XGrabKey(display, XKeysymToKeycode(display, SHORTCUT_KEY), SHORTCUT_MOD | Mod2Mask, DefaultRootWindow(display), True, GrabModeAsync, GrabModeAsync);
		XGrabKey(display, XKeysymToKeycode(display, SHORTCUT_KEY), SHORTCUT_MOD | Mod3Mask, DefaultRootWindow(display), True, GrabModeAsync, GrabModeAsync);
		XGrabKey(display, XKeysymToKeycode(display, SHORTCUT_KEY), SHORTCUT_MOD | LockMask | Mod2Mask, DefaultRootWindow(display), True, GrabModeAsync, GrabModeAsync);
		XGrabKey(display, XKeysymToKeycode(display, SHORTCUT_KEY), SHORTCUT_MOD | LockMask | Mod3Mask, DefaultRootWindow(display), True, GrabModeAsync, GrabModeAsync);
		XGrabKey(display, XKeysymToKeycode(display, SHORTCUT_KEY), SHORTCUT_MOD | Mod2Mask | Mod3Mask, DefaultRootWindow(display), True, GrabModeAsync, GrabModeAsync);
		XGrabKey(display, XKeysymToKeycode(display, SHORTCUT_KEY), SHORTCUT_MOD | LockMask | Mod2Mask | Mod3Mask, DefaultRootWindow(display), True, GrabModeAsync, GrabModeAsync);
	#endif

	XSelectInput(display, window, ExposureMask | ButtonPress);

	XMapWindow(display, window);

	while (1)
	{
		XEvent event;
		XNextEvent(display, &event);

		if (event.type == Expose)
		{
			XClearWindow(display, window);
			for (int i = 0; i < num_of_lines; i++)
				XftDrawStringUtf8(draw, &color, font, padding, line_spacing * i + text_height * (i + 1) + padding, (FcChar8 *)words[i], strlen(words[i]));
		}
		if (event.type == ButtonPress || event.type == KeyPress)
			break;
	}

	for (int i = 0; i < num_of_lines; i++)
		free(words[i]);

	free(words);
	XftDrawDestroy(draw);
	XftColorFree(display, visual, colormap, &color);
	XftFontClose(display, font);
	XCloseDisplay(display);

	exit(EXIT_SUCCESS);
}
