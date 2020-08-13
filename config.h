static char *background_color = "#3e3e3e";
static char *border_color = "#ececec";
static char *font_color = "#ececec";
static char *font_pattern = "Inconsolata:style=Medium:size=12";
static unsigned line_spacing = 5;
static unsigned int padding = 15;

static unsigned int width = 450;
static unsigned int border_size = 2;
static unsigned int pos_x = 30;
static unsigned int pos_y = 60;

enum corners { 
  TOP_LEFT=1,    TOP_CENTER,    TOP_RIGHT, 
  MIDDLE_LEFT,   MIDDLE_CENTER, MIDDLE_RIGHT, 
  BOTTOM_LEFT,   BOTTOM_CENTER, BOTTOM_RIGHT, 
};

enum corners corner = TOP_RIGHT;

static unsigned int duration = 5; /* in seconds */

struct optparse_long longopts[] = {
  {"border",           'b', OPTPARSE_REQUIRED}, // border_size
  {"pos-x",            'x', OPTPARSE_REQUIRED}, // pos_x
  {"pos-y",            'y', OPTPARSE_REQUIRED}, // pos_y
  {"width",            'w', OPTPARSE_REQUIRED}, // width
  {"padding",          'p', OPTPARSE_REQUIRED}, // padding
  {"line-spacing",     'l', OPTPARSE_REQUIRED}, // line_spacing
  {"font",             'f', OPTPARSE_REQUIRED}, // font_pattern
  {"foreground-color", 'C', OPTPARSE_REQUIRED}, // font_color
  {"background-color", 'c', OPTPARSE_REQUIRED}, // background_color
  {"border-color",     'B', OPTPARSE_REQUIRED}, // border_color
  {"expire-time",      't', OPTPARSE_REQUIRED}, // duration
  {"anchor",           'a', OPTPARSE_REQUIRED}, // corner
  {0}
};
