#include "gametank.h"
#include "../drivers/keyboard.h"
#include "../gt/gfx/draw_queue.h"
#include "../gt/gfx/draw_direct.h"
#include "../desktop.h"
#include "../drivers/mouse.h"
#include "../drivers/ps2.h"
#include "../util.h"
#include "../mem.h"
#include "../gen/assets/gfx.h"

#include "../ui/bittext.h"
#include "../drivers/exbus.h"
#include "../drivers/ch37x_usb.h"

#pragma code-name (push, "PROG0")
#pragma rodata-name (push, "PROG0")

extern const char ps2_set2_to_char[256];
extern const char ps2_set2_to_upper_char[256];
extern char usb_storage_device;

typedef struct {
char last_usb_event;
char cursor_y;
char dir_cache[512];
char dir_cache_i;
char dir_cache_count; 
char dir_cache_count_stale;
char pending_dir_open;
char cwd[64];
char cwd_len;
char enum_pending;
char cwd_level;
} fs_browse_app_context;

static char isBreak[MAX_PS2_CARDS] = {0};
static char shiftMask[MAX_PS2_CARDS] = {0};
static keyboard_packet tmp_keyb_packet;
static char nextChar;

#define ctx ((fs_browse_app_context*)my(window_context))

char fs_browse_app_open_count = 0;

static char tmp;
static char *cpy_src, *cpy_dst;

#define FS_BROWSE_APP_W 64
#define FS_BROWSE_APP_H 64

static char subwindow_rect_test() {
    if(!my(window_open)) return 0;

    if(u(mouse_display_x[activeMouse] - (my(window_x)+1)) > u(my(window_w)-2)) return 0;
    if(u(mouse_display_y[activeMouse] - (my(window_y)+4)) > u(my(window_h)-5)) return 0;

    return 1;
}

static void fs_browse_app_handler(char e) {
    char i,k;
    switch(e) {
        case WINDOW_EVENT_DRAW:
            queue_draw_sprite(my(window_x), my(window_y), FS_BROWSE_APP_W, FS_BROWSE_APP_H, 0, 0, my(window_sprite));
            break;
        case WINDOW_EVENT_TICK:
            break;
        case WINDOW_EVENT_LATE_TICK:
            direct_prepare_sprite_ram_array_mode(my(window_sprite));
            if(ch37x_init_called) {
                ch37x_select(usb_storage_device);

                if(ctx->pending_dir_open && (ch37x_last_int_status == CH37X_ERR_OPEN_DIR)) {
                    ch37x_write_cmd(CH376_CMD_SET_FILE_NAME);
                    ch37x_write_data('*');
                    ch37x_write_data(0);
                    ch37x_write_cmd(CH376_CMD_FILE_OPEN);
                }

                if(ctx->last_usb_event != ch37x_read_event_index) {
                    ctx->last_usb_event = ch37x_read_event_index;
                    if(ch37x_last_int_status == CH37X_USB_INT_DISK_READ) {
                        if(ch37x_read_buffer[0] != '.' || ch37x_read_buffer[1] != ' ') {
                            bittext_bg = 7;
                            bittext_color = 32;
                            bittext_set_cursor(1, ctx->cursor_y); 
                            k = 0;
                            for(i = 0; i < 8; ++i) {
                                if(ch37x_read_buffer[i] == 0) break;
                                if(ch37x_read_buffer[i] == ' ') break;
                                bittext_write_char_opaque(ch37x_read_buffer[i]);
                                ctx->dir_cache[ctx->dir_cache_i++] = ch37x_read_buffer[i];
                                ++k;
                            }
                            if(ch37x_read_buffer[8] != ' ') {
                                bittext_write_char_opaque('.');
                                for(i = 8; i < 11; ++i) {
                                    if(ch37x_read_buffer[i] == 0) break;
                                    bittext_write_char_opaque(ch37x_read_buffer[i]);
                                    ctx->dir_cache[ctx->dir_cache_i++] = ch37x_read_buffer[i];
                                    ++k;
                                }
                            }
                            for(; k <= 12; ++k) {
                                bittext_erase_char();
                            }
                            ctx->dir_cache[ctx->dir_cache_i++] = 0;
                            ctx->dir_cache_i &= 0xF0;
                            ctx->dir_cache[ctx->dir_cache_i+15] = ch37x_read_buffer[11];
                            ctx->dir_cache_i += 16;
                            ctx->cursor_y += 8;
                            ++ctx->dir_cache_count;
                        }
                        ch37x_write_cmd(CH376_CMD_FILE_ENUM_GO);
                    } else if((ch37x_last_int_status == CH37X_ERR_MISS_FILE) && (ch37x_prev_cmd == CH376_CMD_FILE_ENUM_GO)) {
                        ctx->enum_pending = 0;
                        bittext_bg = 7;
                        bittext_color = 32;
                        for(i = ctx->dir_cache_count; i <= ctx->dir_cache_count_stale; ++i) {
                            bittext_set_cursor(1, ctx->cursor_y);
                            for(k = 0; k <= 12; ++k) {
                                bittext_erase_char();
                            }
                            ctx->cursor_y += 8;
                        }
                    }
                }
                EXBUS_MODE = EXBUS_IGNORE;
            }
            flagsMirror |= DMA_ENABLE;
            *dma_flags = flagsMirror;
            break;
        case WINDOW_EVENT_MOUSE_CLICK:
            i = (mouse_display_y[activeMouse] - my(window_y) - 8) >> 3;
            if((i < ctx->dir_cache_count) && !ctx->enum_pending) {
                i = i << 4;
                if((ctx->dir_cache[i] == '.') && (ctx->dir_cache[i+1] == '.')) {
                    k = ctx->cwd_len;
                    while(ctx->cwd[k] != '/') --k;
                    ctx->cwd[k] = 0;
                    ctx->cwd_len = k;
                    k = 0;
                    ch37x_select(usb_storage_device);
                    ch37x_write_cmd(CH376_CMD_SET_FILE_NAME);
                    ch37x_write_data('/');
                    if(ctx->cwd[k] != 0) {
                        k = 1;
                        while(ctx->cwd[k] != 0) {
                            if(ctx->cwd[k] == '/') {
                                ch37x_write_data(0);
                                ch37x_write_cmd(CH376_CMD_FILE_OPEN);
                                while(ch37x_poll_for_int() == 0xFF) {}
                                ch37x_write_cmd(CH376_CMD_SET_FILE_NAME);
                            } else {
                                ch37x_write_data(ctx->cwd[k]);
                            }
                            ++k;
                        }
                    }
                    ch37x_write_data(0);
                    ch37x_write_cmd(CH376_CMD_FILE_OPEN);
                    ctx->pending_dir_open = 1;
                    ctx->dir_cache_i = 0;
                    ctx->dir_cache_count_stale = ctx->dir_cache_count;
                    ctx->dir_cache_count = 0;
                    ctx->enum_pending = 1;
                    ctx->cursor_y = 8;
                    --ctx->cwd_level;
                    EXBUS_MODE = EXBUS_IGNORE;
                } else if(ctx->dir_cache[i + 15] & 0x10) {
                    ch37x_select(usb_storage_device);
                    ch37x_write_cmd(CH376_CMD_SET_FILE_NAME);
                    k = ctx->cwd_len;
                    ctx->cwd[k++] = '/';
                    while(ctx->dir_cache[i] != 0) {
                        ch37x_write_data(ctx->dir_cache[i]);
                        ctx->cwd[k++] = ctx->dir_cache[i++];
                    }
                    ctx->cwd[k] = 0;
                    ctx->cwd_len = k;
                    ch37x_write_data(0);
                    ch37x_write_cmd(CH376_CMD_FILE_OPEN);
                    ctx->pending_dir_open = 1;
                    ctx->dir_cache_i = 0;
                    ctx->dir_cache_count_stale = ctx->dir_cache_count;
                    ctx->dir_cache_count = 0;
                    ctx->enum_pending = 1;
                    ctx->cursor_y = 8;
                    ++ctx->cwd_level;
                    EXBUS_MODE = EXBUS_IGNORE;
                }
            }
            break;
        case WINDOW_EVENT_MOUSE_RELEASE:
            break;
        case WINDOW_EVENT_EXIT:
            free_sprite(my(window_sprite));
            mem_free(my(window_context));
            --fs_browse_app_open_count;
            break;
        default:
            break;
    }
}

void fs_browse_app_launch() {
    if(desktop_launch_app(fs_browse_app_handler) != 255) {
        my(window_context) = mem_alloc(sizeof(fs_browse_app_context));
        my(window_sprite) = allocate_sprite(&ASSET__gfx__blank_bmp_load_list);
        my(window_x) = 32 + (fs_browse_app_open_count<<2);
        my(window_y) = 24 + (fs_browse_app_open_count<<2);
        my(window_w) = FS_BROWSE_APP_W;
        my(window_h) = FS_BROWSE_APP_H;

        ctx->last_usb_event = ch37x_read_event_index;
        ctx->cursor_y = 8;
        ctx->dir_cache_i = 0;
        ctx->dir_cache_count = 0;
        ctx->dir_cache_count_stale = 0;
        ctx->pending_dir_open = 0;
        ctx->cwd[0] = '0';
        ctx->cwd_len = 0;
        ctx->cwd_level = 0;
        ctx->enum_pending = 0;
        ++fs_browse_app_open_count;

        if(ch37x_init_called) {
            ch37x_select(usb_storage_device);
            ch37x_write_cmd(CH376_CMD_SET_FILE_NAME);
            ch37x_write_data('/');
            ch37x_write_data('*');
            ch37x_write_data(0);
            ch37x_write_cmd(CH376_CMD_FILE_OPEN);
            ctx->enum_pending = 1;
            EXBUS_MODE = EXBUS_IGNORE;
        }
    }
}