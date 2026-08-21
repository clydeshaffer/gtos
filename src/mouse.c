#include "ps2.h"
#include "util.h"
#include "mouse.h"

#define MOUSE_SHIFT_BITS 2
char mouse_display_x[MAX_MICE] = {64}, mouse_display_y[MAX_MICE] = {64};
int mouse_x[MAX_MICE] = {64 << MOUSE_SHIFT_BITS}, mouse_y[MAX_MICE] = {64 << MOUSE_SHIFT_BITS};

char byteParity[MAX_MICE] = {0};
char mouseStatus[MAX_MICE] = {0};
int mouse_rel_x;
int mouse_rel_y;
char lastMouseRead = 0;

char firstMouse = 255;

char poll_mouse(char index) {
    ps2_select(index);
    if(ps2_data_ready()) {
        lastMouseRead = read_byte();

        mouse_display_x[index] = mouse_x[index] >> MOUSE_SHIFT_BITS;
        mouse_display_y[index] = mouse_y[index] >> MOUSE_SHIFT_BITS;

        if(byteParity[index] == 0) {
            mouseStatus[index] = lastMouseRead;
        } else if(byteParity[index] == 1) {
            mouse_rel_x = lastMouseRead;
            if(mouseStatus[index] & 16) {
                mouse_rel_x |= 0xFF00;
            }
            mouse_x[index] += mouse_rel_x;
            if(mouse_x[index] < 0) mouse_x[index] = 0;
            if(mouse_x[index] > (127 << MOUSE_SHIFT_BITS)) mouse_x[index] = (127 << MOUSE_SHIFT_BITS);
        } else if(byteParity[index] == 2) {
            mouse_rel_y = lastMouseRead;
            if(mouseStatus[index] & 32) {
                mouse_rel_y |= 0xFF00;
            }
            mouse_y[index] -= mouse_rel_y;
            if(mouse_y[index] < 0) mouse_y[index] = 0;
            if(mouse_y[index] > (127 << MOUSE_SHIFT_BITS)) mouse_y[index] = (127 << MOUSE_SHIFT_BITS);
        }

        byteParity[index]++;
        if(byteParity[index] == 3) byteParity[index] = 0;
        return 1;
    } else {
        return 0;
    }
}