
#include "bittext.h"
#include "../drivers/ps2.h"
#include "../drivers/keyboard.h"

#pragma code-name (push, "PROG0")
#pragma rodata-name (push, "PROG0")

extern const char ps2_set2_to_char[256];
extern const char ps2_set2_to_upper_char[256];

static keyboard_packet tmp_keyb_packet;
static char isBreak[MAX_PS2_CARDS] = {0};
static char shiftMask[MAX_PS2_CARDS] = {0};

char* lineentry_buf;
char lineentry_len;
char lineentry_x;
char lineentry_y;
char* lineentry_cursor;

char lineentry_update() {
    char nextChar;
    while(keyboard_buf_count()) {
        tmp_keyb_packet = pop_keyboard();
        if(tmp_keyb_packet.data == 0xF0) {
            isBreak[tmp_keyb_packet.src] = 1;
        } else {
            if(isBreak[tmp_keyb_packet.src]) {
                
            } else {
                if(!!shiftMask[tmp_keyb_packet.src]) {
                    nextChar = ps2_set2_to_upper_char[tmp_keyb_packet.data];
                } else {
                    nextChar = ps2_set2_to_char[tmp_keyb_packet.data];
                }

                if(nextChar) {
                    if(nextChar == '\n') {
                        //lineentry_buf[*lineentry_cursor] = 0;
                        return 1;
                    } else {
                        if(nextChar == '\b') {
                            if((*lineentry_cursor) > 0) {
                                if((*lineentry_cursor) < lineentry_len) {
                                    lineentry_buf[*lineentry_cursor] = ' ';
                                    bittext_erase_char();
                                }
                                *lineentry_cursor = *lineentry_cursor - 1;
                                bittext_set_cursor(((*lineentry_cursor) << 2) + lineentry_x, lineentry_y);
                                bittext_erase_char();
                            }
                        } else {
                            bittext_set_cursor(((*lineentry_cursor) << 2) + lineentry_x, lineentry_y);
                            bittext_write_char_opaque(nextChar);
                            lineentry_buf[*lineentry_cursor] = nextChar;
                            if((*lineentry_cursor) < lineentry_len) {
                                *lineentry_cursor = *lineentry_cursor + 1;    
                            }
                        }
                    }
                }
            }

            if(tmp_keyb_packet.data == 0x12) {
                shiftMask[tmp_keyb_packet.src] &= ~1;
                shiftMask[tmp_keyb_packet.src] |= !isBreak[tmp_keyb_packet.src];
            } else if(tmp_keyb_packet.data == 0x59) {
                shiftMask[tmp_keyb_packet.src] &= ~2;
                shiftMask[tmp_keyb_packet.src] |= !isBreak[tmp_keyb_packet.src] << 1;
            }
            isBreak[tmp_keyb_packet.src] = 0;
        }
    }
    return 0;
}