#include "gametank.h"
#include "../keyboard.h"
#include "../gt/gfx/draw_queue.h"
#include "../gt/gfx/draw_direct.h"
#include "../desktop.h"
#include "../mouse.h"
#include "../ps2.h"
#include "../util.h"
#include "../mem.h"
#include "../gen/assets/gfx.h"

#pragma code-name (push, "PROG0")
#pragma rodata-name (push, "PROG0")

extern const char ps2_set2_to_char[256];
extern const char ps2_set2_to_upper_char[256];

typedef struct {
char cursor_x;
char cursor_y;
} text_edit_app_context;

static char isBreak[MAX_PS2_CARDS] = {0};
static char shiftMask[MAX_PS2_CARDS] = {0};
static keyboard_packet tmp_keyb_packet;
static char nextChar;

#define ctx ((text_edit_app_context*)my(window_context))

char text_edit_app_open_count = 0;

static char tmp;
static char *cpy_src, *cpy_dst;

#define TEXT_EDIT_APP_W 82
#define TEXT_EDIT_APP_H 77

static char subwindow_rect_test() {
    if(!my(window_open)) return 0;

    if(u(mouse_display_x[activeMouse] - (my(window_x)+1)) > u(my(window_w)-2)) return 0;
    if(u(mouse_display_y[activeMouse] - (my(window_y)+4)) > u(my(window_h)-5)) return 0;

    return 1;
}

static void text_edit_app_handler(char e) {
    switch(e) {
        case WINDOW_EVENT_DRAW:
            queue_draw_sprite(my(window_x), my(window_y), TEXT_EDIT_APP_W, TEXT_EDIT_APP_H, 0, 0, my(window_sprite));
            break;
        case WINDOW_EVENT_TICK:
            break;
        case WINDOW_EVENT_LATE_TICK:
            direct_prepare_sprite_ram_array_mode(my(window_sprite));
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
                                ctx->cursor_x = 0;
                                ctx->cursor_y += 8;
                            } else {
                                tmp = 0;
                                if(nextChar == '\b') {
                                    if(ctx->cursor_x < 4) {
                                        ctx->cursor_x = 76;
                                        ctx->cursor_y -= 8;
                                    } else {
                                        ctx->cursor_x -= 4;
                                    }
                                    nextChar -= ' ';
                                    cpy_src = &(vram[12288]);
                                } else {
                                    nextChar -= ' ';
                                    cpy_src = &(vram[12288 + (((nextChar & 0xE0) << 5)) + ((nextChar & 31) << 2)]);
                                }
                                cpy_dst = &(vram[(ctx->cursor_y << 7) + ctx->cursor_x+1]);                                
                                
                                do {
                                    *cpy_dst = *cpy_src;
                                    ++cpy_dst;
                                    ++cpy_src;
                                    if((tmp & 3) == 3) {
                                        cpy_dst += 124;
                                        cpy_src += 124;
                                    }
                                    ++tmp;
                                } while(tmp < 30);
                                nextChar += ' ';
                                if(nextChar != '\b') {
                                    ctx->cursor_x+=4;
                                    if(ctx->cursor_x == 80) {
                                        ctx->cursor_x = 0;
                                        ctx->cursor_y += 8;
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
            break;
        case WINDOW_EVENT_MOUSE_CLICK:
            break;
        case WINDOW_EVENT_MOUSE_RELEASE:
            break;
        case WINDOW_EVENT_EXIT:
            free_sprite(my(window_sprite));
            mem_free(my(window_context));
            --text_edit_app_open_count;
            break;
        default:
            break;
    }
}

void text_edit_app_launch() {
    if(desktop_launch_app(text_edit_app_handler) != 255) {
        my(window_context) = mem_alloc(sizeof(text_edit_app_context));
        my(window_sprite) = allocate_sprite(&ASSET__gfx__textedit_bmp_load_list);
        my(window_x) = 32 + (text_edit_app_open_count<<2);
        my(window_y) = 24 + (text_edit_app_open_count<<2);
        my(window_w) = TEXT_EDIT_APP_W;
        my(window_h) = TEXT_EDIT_APP_H;

        ctx->cursor_x = 0;
        ctx->cursor_y = 4;
        ++text_edit_app_open_count;
    }
}