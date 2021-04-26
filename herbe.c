#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>
#include <errno.h>
#include <fcntl.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#include "config.h"

#define EXIT_ACTION 0
#define EXIT_FAIL 1
#define EXIT_DISMISS 2

Display *display;
XftFont *font;
Window window;
int num_of_lines;
char **lines;
int exit_code = EXIT_DISMISS;

struct mq_object {
	pid_t pid;
	long timestamp;
	char  buffer[1024];
};
long lastTimestamp;

static void die(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(EXIT_FAIL);
}

void read_y_offset(unsigned int **offset, int *id) {
	int shm_id = shmget(8432, sizeof(unsigned int), IPC_CREAT | 0660);
	if (shm_id == -1) die("shmget failed");

	*offset = (unsigned int *)shmat(shm_id, 0, 0);
	if (*offset == (unsigned int *)-1) die("shmat failed\n");
	*id = shm_id;
}

void free_y_offset(int id) {
	shmctl(id, IPC_RMID, NULL);
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

void freeLines() {
	if(lines) {
		for (int i = 0; i < num_of_lines; i++)
			free(lines[i]);
		free(lines);
	}
}

void constructLines(char* strList[], int numberOfStrings) {
	freeLines();
	int max_text_width = width - 2 * padding;
	num_of_lines = 0;
	int lines_size = 5;
	lines = malloc(lines_size * sizeof(char *));
	if (!lines)
		die("malloc failed");

	for (int i = 0; i < numberOfStrings; i++)
	{
		for (unsigned int eol = get_max_len(strList[i], font, max_text_width); eol; strList[i] += eol, num_of_lines++, eol = get_max_len(strList[i], font, max_text_width))
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

			strncpy(lines[num_of_lines], strList[i], eol);
			lines[num_of_lines][eol] = '\0';
		}
	}
}

void reload(union sigval sv);
void readAllEvents(mqd_t mqd) {

	struct sigevent event = {.sigev_notify=SIGEV_THREAD, .sigev_signo=SIGHUP, .sigev_value.sival_int=mqd, .sigev_notify_function=reload};
	if(mq_notify(mqd, &event) == -1) {
		perror("mq_notify failed");
		exit(1);
	}
	struct mq_object object;
	while(1) {
		int ret = mq_receive(mqd, (char*)&object, sizeof(object), NULL);
		if(ret==-1) {
			if(errno == EAGAIN)
				return;
			perror("mq_receive");
			exit(1);
		}
		if(object.timestamp && lastTimestamp > object.timestamp)
			return;
		if(object.timestamp)
			lastTimestamp = object.timestamp;
		char *buffer = object.buffer;

		constructLines(&buffer, 1);
		kill(object.pid, SIGTERM);
	}
}
void reload(union sigval sv) {
	// we've already timed out
	if(alarm(duration) == 0)
		return;
	readAllEvents(sv.sival_int);
	XEvent event;
	event.type = Expose;
	XSendEvent(display, window, 0, 0, &event);
	XFlush(display);
}

void expire(int sig)
{
	XEvent event;
	event.type = ButtonPress;
	event.xbutton.button = (sig == SIGUSR2) ? (ACTION_BUTTON) : (DISMISS_BUTTON);
	XSendEvent(display, window, 0, 0, &event);
	XFlush(display);
}


void exitSuccess() {
       exit(0);
}

int main(int argc, char *argv[])
{
	if (argc == 1)
	{
		die("Usage: %s body", argv[0]);
	}

	const char* id =getenv("HERBE_ID");
	mqd_t mqd=-1;
	if(id) {
		struct mq_attr attr = { .mq_maxmsg = 10, .mq_msgsize = sizeof(struct mq_object) };
		mqd = mq_open(id, O_RDWR|O_CREAT|O_NONBLOCK, 0722, &attr);
		if(mqd==-1){
			perror("mq_open");
			die("mq_open");
		}
		while (1) {
			if(flock(mqd, LOCK_EX|LOCK_NB) == 0) {
				// if we get the lock, register for events
				break;
			}
			if(errno != EWOULDBLOCK) {
				perror("flock");
				exit(1);
			}
			// someone else is listening for events
			char* ts_str = getenv("NOTIFICATION_ID");
			lastTimestamp = ts_str?atol(ts_str):0;
			struct mq_object object = {getpid(), lastTimestamp, {0}};
			char *buffer=object.buffer;
			for(int i=1;i<argc;i++) {
				strcat(buffer, argv[i]);
				strcat(buffer, "\n");
			}
			signal(SIGTERM, exitSuccess);
			if(mq_send(mqd, (char*)&object, sizeof(object), 1)==-1) {
				perror("mq_send");
				exit(1);
			}
			signal(SIGALRM, SIG_IGN);
			alarm(1);
			pause();
		}
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

	int screen_width = DisplayWidth(display, screen);
	int screen_height = DisplayHeight(display, screen);

	XSetWindowAttributes attributes;
	attributes.override_redirect = True;
	XftColor color;
	XftColorAllocName(display, visual, colormap, background_color, &color);
	attributes.background_pixel = color.pixel;
	XftColorAllocName(display, visual, colormap, border_color, &color);
	attributes.border_pixel = color.pixel;
	font = XftFontOpenName(display, screen, font_pattern);

	constructLines(argv+1, argc-1);

	//unsigned int x = pos_x;
	//unsigned int y = pos_y;
	int y_offset_id;
	unsigned int *y_offset;
	read_y_offset(&y_offset, &y_offset_id);

	unsigned int text_height = font->ascent - font->descent;
	unsigned int height = (num_of_lines - 1) * line_spacing + num_of_lines * text_height + 2 * padding;
	unsigned int x = pos_x;
	unsigned int y = pos_y + *y_offset;
	unsigned int used_y_offset = (*y_offset) += height + padding;

	if (corner == TOP_RIGHT || corner == BOTTOM_RIGHT)
		x = screen_width - width - border_size * 2 - x;
		//x = screen_width - width - border_size * 2 - pos_x;

	if (corner == BOTTOM_LEFT || corner == BOTTOM_RIGHT)
		y = screen_height - height - border_size * 2 - y;
		//y = screen_height - height - border_size * 2 - pos_y;

	window = XCreateWindow(display, RootWindow(display, screen), x, y, width, height, border_size, DefaultDepth(display, screen),
						   CopyFromParent, visual, CWOverrideRedirect | CWBackPixel | CWBorderPixel, &attributes);

	XftDraw *draw = XftDrawCreate(display, window, visual, colormap);
	XftColorAllocName(display, visual, colormap, font_color, &color);

	XSelectInput(display, window, ExposureMask | ButtonPress);
	XMapWindow(display, window);

	sigaction(SIGUSR1, &act_expire, 0);
	sigaction(SIGUSR2, &act_expire, 0);

	if (duration != 0)
		alarm(duration);

	if(id) {
		readAllEvents(mqd);
	}

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

	if (used_y_offset == *y_offset) free_y_offset(y_offset_id);
	freeLines();

	XftDrawDestroy(draw);
	XftColorFree(display, visual, colormap, &color);
	XftFontClose(display, font);
	XCloseDisplay(display);

	if(id) {
		mq_close(mqd);
	}

	return exit_code;
}
