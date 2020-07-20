#include <xcb/xcb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
	xcb_connection_t *conn;
	xcb_screen_t *screen;
	xcb_window_t window;
	uint32_t mask;
	uint32_t values[2];

	conn = xcb_connect(NULL, NULL);
	if (xcb_connection_has_error(conn)) {
		printf("Cannot open display\n");
		exit(EXIT_FAILURE);
	}

	screen = xcb_setup_roots_iterator( xcb_get_setup(conn) ).data;

	window = xcb_generate_id(conn);

	int width = 200;
	int height = 60;

	uint32_t back = 0xFFFFFF;

	int sw = screen->width_in_pixels;
	int sh = screen->height_in_pixels;

	int padding_x = 30;
	int padding_y = 50;

	int pos_x = sw - width - padding_x;
	int pos_y = padding_y;

	mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT;

	values[0] = back;
	values[1] = 1;

	xcb_create_window(conn, screen->root_depth, 
			window, screen->root, pos_x, pos_y, width, height, 1,
			XCB_WINDOW_CLASS_INPUT_OUTPUT, 
			screen->root_visual, mask, values);

	xcb_map_window(conn, window);
	xcb_flush(conn);

	sleep(30);

	xcb_disconnect(conn);

	exit(EXIT_SUCCESS);
}
