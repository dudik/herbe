#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"

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

void parse_commandline(struct optparse *options)
{

  int option;

  while ((option = optparse_long(options, longopts, NULL)) != -1)
  {
    switch (option)
    {
	    case 'b':
	      border_size=atoi(options->optarg);
	      break;
	    case 'x':
	      pos_x=atoi(options->optarg);
	      break;
	    case 'y':
	      pos_y=atoi(options->optarg);
	      break;
	    case 'w':
	      width=atoi(options->optarg);
	      break;
	    case 'p':
	      padding=atoi(options->optarg);
	      break;
	    case 'l':
	      line_spacing=atoi(options->optarg);
	      break;
	    case 'f':
	      font_pattern=options->optarg;
	      break;
	    case 'C':
	      font_color=options->optarg;
	      break;
	    case 'c':
	      background_color=options->optarg;
	      break;
	    case 'B':
	      border_color=options->optarg;
	      break;
	    case 't':
	      duration=atoi(options->optarg);
	      break;
	    case 'a':
	      corner=atoi(options->optarg);
	      break;
	    case '?':
	      die(options->errmsg);
	  }
	}
}

int main(int argc, char *argv[])
{
	if (argc == 1)
		die("%s",
                "Usage: herbe [OPTION ARG...] body \n\n"
                "OPTIONS:\n"
                "long               | short | default ARG  \n"
                "-------------------|-------|-----------\n"
                "--border           |  -b   | 2\n"
                "--pos-x            |  -x   | 30\n"
                "--pos-y            |  -y   | 60\n"
                "--width            |  -w   | 450\n"
                "--padding          |  -p   | 15\n"
                "--line-spacing     |  -l   | 5\n"
                "--font             |  -f   | 'Inconsolata:style=Medium:size=12'\n"
                "--foreground-color |  -C   | '#ececec'\n"
                "--background-color |  -c   | '#3e3e3e'\n"
                "--border-color     |  -B   | '#ececec'\n"
                "--expire-time      |  -t   | 5\n"
                "--anchor           |  -a   | 3\n"
        );

	struct optparse options;

	optparse_init(&options, argv);
	parse_commandline(&options);

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

	char *arg; // non option arguments
	while ((arg = optparse_arg(&options)))
	{
		char *body = arg;

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

	if (corner == TOP_RIGHT || corner ==  BOTTOM_RIGHT || corner ==  MIDDLE_RIGHT)
		x = screen_width - width - border_size * 2 - pos_x;

	else if (corner == TOP_CENTER || corner ==  MIDDLE_CENTER || corner ==  BOTTOM_CENTER)
		x = (screen_width - width)/2;

	if (corner == BOTTOM_LEFT || corner ==  BOTTOM_RIGHT || corner ==  BOTTOM_CENTER)
		y = screen_height - height - border_size * 2 - pos_y;

	else if (corner == MIDDLE_LEFT || corner ==  MIDDLE_CENTER || corner ==  MIDDLE_RIGHT)
		y = (screen_height - height)/2;

	window = XCreateWindow(display, RootWindow(display, screen), x, y, width, height, border_size, DefaultDepth(display, screen), CopyFromParent, visual,
						   CWOverrideRedirect | CWBackPixel | CWBorderPixel, &attributes);

	XftDraw *draw = XftDrawCreate(display, window, visual, colormap);
	XftColorAllocName(display, visual, colormap, font_color, &color);

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
		if (event.type == ButtonPress)
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
