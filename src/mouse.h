#define MAX_MICE 8

extern char mouse_display_x[MAX_MICE], mouse_display_y[MAX_MICE];
extern char mouseStatus[MAX_MICE];
extern char firstMouse;
char mouse_parity(char index);
char poll_mouse(char index);