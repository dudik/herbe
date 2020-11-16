#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>
#include <X11/extensions/Xrandr.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <semaphore.h>

#include "config.h"

#define EXIT_ACTION 0
#define EXIT_FAIL 1
#define EXIT_DISMISS 2

Display *display;
Window window;
int exit_code = EXIT_DISMISS;

static void die(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(EXIT_FAIL);
}

int get_max_len(char *string, XftFont *font, int max_text_width)
{
	int eol = strlen(string);
	XGlyphInfo info;
	XftTextExtentsUtf8(display, font, (FcChar8 *)string, eol, &info);

	if (info.width > max_text_width)
	{
		eol = max_text_width / font->max_advance_width;
		info.width = 0;

		while (info.width < max_text_width)
		{
			eol++;
			XftTextExtentsUtf8(display, font, (FcChar8 *)string, eol, &info);
		}

		eol--;
	}

	for (int i = 0; i < eol; i++)
		if (string[i] == '\n')
		{
			string[i] = ' ';
			return ++i;
		}

	if (info.width <= max_text_width)
		return eol;

	int temp = eol;

	while (string[eol] != ' ' && eol)
		--eol;

	if (eol == 0)
		return temp;
	else
		return ++eol;
}

void expire(int sig)
{
	XEvent event;
	event.type = ButtonPress;
	event.xbutton.button = (sig == SIGUSR2) ? (ACTION_BUTTON) : (DISMISS_BUTTON);
	XSendEvent(display, window, 0, 0, &event);
	XFlush(display);
}

int main(int argc, char *argv[])
{
	if (argc == 1)
	{
		sem_unlink("/herbe");
		die("Usage: %s body", argv[0]);
	}

	struct sigaction act_expire, act_ignore;

	act_expire.sa_handler = expire;
	act_expire.sa_flags = SA_RESTART;
	sigemptyset(&act_expire.sa_mask);

	act_ignore.sa_handler = SIG_IGN;
	act_ignore.sa_flags = 0;
	sigemptyset(&act_ignore.sa_mask);

	sigaction(SIGALRM, &act_expire, 0);
	sigaction(SIGTERM, &act_expire, 0);
	sigaction(SIGINT, &act_expire, 0);

	sigaction(SIGUSR1, &act_ignore, 0);
	sigaction(SIGUSR2, &act_ignore, 0);

	if (!(display = XOpenDisplay(0)))
		die("Cannot open display");

	int screen = DefaultScreen(display);
	Visual *visual = DefaultVisual(display, screen);
	Colormap colormap = DefaultColormap(display, screen);

	int screen_x = 0;
	int screen_y = 0;
	int screen_width = DisplayWidth(display, screen);
	int screen_height = DisplayHeight(display, screen);
	if(use_primary_monitor) {
		int nMonitors;
		XRRMonitorInfo* info = XRRGetMonitors(display, RootWindow(display, screen), 1, &nMonitors);
		for(int i = 0; i < nMonitors; i++) {
			if(info[i].primary) {
				screen_x = info[i].x;
				screen_y = info[i].y;
				screen_width = info[i].width;
				screen_height = info[i].height;
			}
		}
	}

	XSetWindowAttributes attributes;
	attributes.override_redirect = True;
	XftColor color;
	XftColorAllocName(display, visual, colormap, background_color, &color);
	attributes.background_pixel = color.pixel;
	XftColorAllocName(display, visual, colormap, border_color, &color);
	attributes.border_pixel = color.pixel;

	int num_of_lines = 0;
	int max_text_width = width - 2 * padding;
	int lines_size = 5;
	char **lines = malloc(lines_size * sizeof(char *));
	if (!lines)
		die("malloc failed");

	XftFont *font = XftFontOpenName(display, screen, font_pattern);

	for (int i = 1; i < argc; i++)
	{
		for (unsigned int eol = get_max_len(argv[i], font, max_text_width); eol; argv[i] += eol, num_of_lines++, eol = get_max_len(argv[i], font, max_text_width))
		{
			if (lines_size <= num_of_lines)
			{
				lines = realloc(lines, (lines_size += 5) * sizeof(char *));
				if (!lines)
					die("realloc failed");
			}

			lines[num_of_lines] = malloc((eol + 1) * sizeof(char));
			if (!lines[num_of_lines])
				die("malloc failed");

			strncpy(lines[num_of_lines], argv[i], eol);
			lines[num_of_lines][eol] = '\0';
		}
	}

	unsigned int x = screen_x + pos_x;
	unsigned int y = screen_y + pos_y;
	unsigned int text_height = font->ascent - font->descent;
	unsigned int height = (num_of_lines - 1) * line_spacing + num_of_lines * text_height + 2 * padding;

	if (corner == TOP_RIGHT || corner == BOTTOM_RIGHT)
		x = screen_x + screen_width - width - border_size * 2 - pos_x;

	if (corner == BOTTOM_LEFT || corner == BOTTOM_RIGHT)
		y = screen_y + screen_height - height - border_size * 2 - pos_y;

	window = XCreateWindow(display, RootWindow(display, screen), x, y, width, height, border_size, DefaultDepth(display, screen),
						   CopyFromParent, visual, CWOverrideRedirect | CWBackPixel | CWBorderPixel, &attributes);

	XftDraw *draw = XftDrawCreate(display, window, visual, colormap);
	XftColorAllocName(display, visual, colormap, font_color, &color);

	XSelectInput(display, window, ExposureMask | ButtonPress);
	XMapWindow(display, window);

	sem_t *mutex = sem_open("/herbe", O_CREAT, 0644, 1);
	sem_wait(mutex);

	sigaction(SIGUSR1, &act_expire, 0);
	sigaction(SIGUSR2, &act_expire, 0);

	if (duration != 0)
		alarm(duration);

	for (;;)
	{
		XEvent event;
		XNextEvent(display, &event);

		if (event.type == Expose)
		{
			XClearWindow(display, window);
			for (int i = 0; i < num_of_lines; i++)
				XftDrawStringUtf8(draw, &color, font, padding, line_spacing * i + text_height * (i + 1) + padding,
								  (FcChar8 *)lines[i], strlen(lines[i]));
		}
		else if (event.type == ButtonPress)
		{
			if (event.xbutton.button == DISMISS_BUTTON)
				break;
			else if (event.xbutton.button == ACTION_BUTTON)
			{
				exit_code = EXIT_ACTION;
				break;
			}
		}
	}

	sem_post(mutex);
	sem_close(mutex);

	for (int i = 0; i < num_of_lines; i++)
		free(lines[i]);

	free(lines);
	XftDrawDestroy(draw);
	XftColorFree(display, visual, colormap, &color);
	XftFontClose(display, font);
	XCloseDisplay(display);

	return exit_code;
}