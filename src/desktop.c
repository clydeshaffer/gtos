#include "desktop.h"
#include "mouse.h"
#include "keyboard.h"
#include "scancodes.h"
#include "ps2.h"
#include "gt/gfx/draw_queue.h"
#include "gt/gfx/draw_direct.h"
#include "gt/feature/random/random.h"
#include "util.h"
#include "gen/assets/gfx.h"

#include "apps/draw.h"
#include "apps/spikeball.h"
#include "apps/mines.h"
#include "apps/mplayer.h"
#include "apps/textedit.h"

#include "gen/assets/gfx/icons.json.h"

#define MAX_ICONS 7
char icons_x[MAX_ICONS] = { 24, 64, 72, 32, 80, 90, 18};
char icons_y[MAX_ICONS] = { 24, 64, 36, 72, 90, 64, 64};
char icons_f[MAX_ICONS] = {
    ICONS_TAG_TRASH_START,
    ICONS_TAG_FOLDER_START,
    ICONS_TAG_EXE_START,
    ICONS_TAG_EXE_START,
    ICONS_TAG_PAINTBRUSH_START,
    ICONS_TAG_SPIKEBALL_START,
    ICONS_TAG_MUSIC_START
};

char icons_app[MAX_ICONS] = {0, 0, 2, 5, 1, 3, 4};

#define MODIFIER_R_ALT 1
#define MODIFIER_L_ALT 2
#define MODIFIER_R_SHIFT 4
#define MODIFIER_L_SHIFT 8
#define MODIFIER_R_CTRL 16
#define MODIFIER_L_CTRL 32
static char modifier_key_mask[MAX_PS2_CARDS] = {0, 0, 0, 0};
static char keycode_isBreak[MAX_PS2_CARDS] = {0, 0, 0, 0};

char dragging_index[MAX_PS2_CARDS] = {255, 255, 255, 255};
char dragged_app[MAX_PS2_CARDS] = {0};
char drag_rel_x[MAX_PS2_CARDS] = {0};
char drag_rel_y[MAX_PS2_CARDS] = {0};
char frames_since_click[MAX_PS2_CARDS] = {0, 0, 0, 0};
static char oldMouseStatus[MAX_PS2_CARDS];
static char alt_tab_depth[MAX_PS2_CARDS] = {2, 2, 2, 2};

SpriteSlot bg_sprite;
SpriteSlot icons_sprite;

char app_to_launch = 0;
char current_app;
char app_draw_index;
char draw_order_count = 0;

char foreground_app = 0xFF;
char window_open[MAX_APPS];
char window_draw_order[MAX_APPS];
char window_x[MAX_APPS];
char window_y[MAX_APPS];
char window_w[MAX_APPS];
char window_h[MAX_APPS];
void* window_context[MAX_APPS];

char last_window_clicked = MAX_APPS;
SpriteSlot window_sprite[MAX_APPS];
char window_to_bump = 0xFF;
char window_to_close = 0xFF;
char activeMouse;

void (*window_handler[MAX_APPS]) (char);
static char tmp = 0;

static keyboard_packet tmp_key_packet;

void remove_from_draw_order(char appslot) {
    static char hit;
    hit = 0xFF;
    for(app_draw_index = 0; app_draw_index < MAX_APPS; ++app_draw_index) {
        if(window_draw_order[app_draw_index] == appslot) {
            hit = app_draw_index;
            break;
        }
    }
    if(hit == 0xFF) return;
    for(app_draw_index = hit; app_draw_index < MAX_APPS-1; ++app_draw_index) {
        window_draw_order[app_draw_index] = window_draw_order[app_draw_index+1];
    }
    window_draw_order[MAX_APPS-1] = 0xFF;
    --draw_order_count;
    if(appslot == foreground_app) {
        foreground_app = 0xFF;
    }
}

void bump_to_top(char appslot) {
    remove_from_draw_order(appslot);
    for(app_draw_index = 0; app_draw_index < MAX_APPS; ++app_draw_index) {
        if(window_draw_order[app_draw_index] == 0xFF) {
            window_draw_order[app_draw_index] = appslot;
            ++draw_order_count;
            foreground_app = appslot;
            return;
        }
    }
}

char window_rect_test(char wi) {
    if(!window_open[wi]) return 0;
    if(u(mouse_display_x[activeMouse] - window_x[wi]) > window_w[wi]) return 0;
    if(u(mouse_display_y[activeMouse] - window_y[wi]) > window_h[wi]) return 0;
    return 1;
}

void desktop_init() {
    bg_sprite = allocate_sprite(&ASSET__gfx__desktop_bmp_load_list);
    icons_sprite = allocate_sprite(&ASSET__gfx__icons_bmp_load_list);
    set_sprite_frametable(icons_sprite, ASSET__gfx__icons_json);
    for(app_draw_index = 0; app_draw_index < MAX_APPS; ++app_draw_index) {
        window_draw_order[app_draw_index] = 0xFF;
    }
}

char desktop_launch_app(void(*handler)(char)) {
    for(current_app = 0; current_app < MAX_APPS; ++current_app) {
        if(!window_open[current_app]) {
            window_open[current_app] = 1;
            window_handler[current_app] = handler;
            for(app_draw_index = 0; app_draw_index < MAX_APPS; ++app_draw_index) {
                if(window_draw_order[app_draw_index] == 0xFF) {
                    window_draw_order[app_draw_index] = current_app;
                    ++draw_order_count;
                    foreground_app = current_app;
                    return current_app;
                }
            }
            //should not be possible to get past this inner for loop
        }
    }
    return 255;
}

void desktop_update() {
    static char tmp_mod_code;
    activeMouse = firstMouse;
    rnd();

    for(tmp = 0; tmp < keyboard_buf_count(); ++tmp) {
        tmp_key_packet = pop_keyboard();
        if(tmp_key_packet.data == 0xF0) {
            keycode_isBreak[tmp_key_packet.src] = 0xFF;
        } else {
            if(!keycode_isBreak[tmp_key_packet.src]) {
                if(tmp_key_packet.data == SCANCODE_TAB) {
                    if(draw_order_count > 1) {
                        if(modifier_key_mask[tmp_key_packet.src] & (MODIFIER_L_ALT | MODIFIER_R_ALT)) {
                            tmp_mod_code = draw_order_count-alt_tab_depth[tmp_key_packet.src];
                            foreground_app = window_draw_order[tmp_mod_code];
                            window_draw_order[tmp_mod_code] = window_draw_order[draw_order_count-1];
                            window_draw_order[draw_order_count-1] = foreground_app;
                            if(alt_tab_depth[tmp_key_packet.src] == draw_order_count) {
                                alt_tab_depth[tmp_key_packet.src] = 2;
                            } else {
                                ++alt_tab_depth[tmp_key_packet.src];
                            }
                        }
                    }
                }
            }
            tmp_mod_code = 0;
            switch(tmp_key_packet.data) {
                case SCANCODE_L_ALT:
                tmp_mod_code = MODIFIER_L_ALT;
                break;
                default:
                break;
            }
            if(tmp_mod_code) {
                modifier_key_mask[tmp_key_packet.src] = (~tmp_mod_code & modifier_key_mask[tmp_key_packet.src]) | (~keycode_isBreak[tmp_key_packet.src] & tmp_mod_code);
                if(!(modifier_key_mask[tmp_key_packet.src] & (MODIFIER_L_ALT | MODIFIER_R_ALT))) {
                    alt_tab_depth[tmp_key_packet.src] = 2;
                }
            }

            keycode_isBreak[tmp_key_packet.src] = 0;
        }
        push_keyboard(tmp_key_packet);
    }

    for(activeMouse = 0; activeMouse < ps2_device_count; ++activeMouse) {
        if(ps2_device_types[activeMouse] != PS2_DEVICE_TYPE_BASIC_MOUSE) {
            continue;
        }
        if(mouseStatus[activeMouse] & (~(oldMouseStatus[activeMouse])) & 3) {
            //Iterate backwards from draw order
            for(app_draw_index = MAX_APPS-1; app_draw_index != 255; --app_draw_index) {
                if(window_draw_order[app_draw_index] == 0xFF) continue;
                current_app = window_draw_order[app_draw_index];
                if(window_rect_test(current_app)) {
                    if(u(mouse_display_y[activeMouse] - window_y[current_app]) < 4) {
                        if(u(mouse_display_x[activeMouse] - window_x[current_app]) < 3) {
                            window_handler[current_app](WINDOW_EVENT_EXIT);
                            window_open[current_app] = 0;
                            window_to_close = current_app;
                        } else {
                            dragging_index[activeMouse] = 'w';
                            dragged_app[activeMouse] = current_app;
                            drag_rel_x[activeMouse] = window_x[current_app] - mouse_display_x[activeMouse];
                            drag_rel_y[activeMouse] = window_y[current_app] - mouse_display_y[activeMouse];
                            window_to_bump = current_app;
                        }
                    } else {
                        window_to_bump = current_app;
                        last_window_clicked = current_app;
                        if(mouseStatus[activeMouse] & (~(oldMouseStatus[activeMouse])) & 1)
                            window_handler[current_app](WINDOW_EVENT_MOUSE_CLICK);
                        if(mouseStatus[activeMouse] & (~(oldMouseStatus[activeMouse]) & 2))
                            window_handler[current_app](WINDOW_EVENT_RIGHT_CLICK);
                    }
                    break;
                }
            }

            if(app_draw_index == 255) {
                for(tmp = MAX_ICONS-1; tmp != 0xFF; --tmp) {
                    if(icons_f[tmp]) {
                        if(cabs(icons_x[tmp] - mouse_display_x[activeMouse]) < 8) {
                            if(cabs(icons_y[tmp] - mouse_display_y[activeMouse]) < 8) {
                                if(icons_app[tmp] && (frames_since_click[activeMouse] < 15)) {
                                    app_to_launch = icons_app[tmp];
                                    break;
                                } else {
                                    dragging_index[activeMouse] = tmp;
                                    drag_rel_x[activeMouse] = icons_x[tmp] - mouse_display_x[activeMouse];
                                    drag_rel_y[activeMouse] = icons_y[tmp] - mouse_display_y[activeMouse];
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            if(window_to_close != 0xFF) {
                remove_from_draw_order(window_to_close);
                window_to_close = 0xFF;
            }

            if(window_to_bump != 0xFF) {
                bump_to_top(window_to_bump);
                window_to_bump = 0xFF;
            }

            frames_since_click[activeMouse] = 0;
        } else if((~(mouseStatus[activeMouse])) & (oldMouseStatus[activeMouse]) & 1) {
            if(last_window_clicked < MAX_APPS) {
                window_handler[last_window_clicked](WINDOW_EVENT_MOUSE_RELEASE);
                last_window_clicked = MAX_APPS;
            } else if((dragging_index[activeMouse] != 0) && (dragging_index[activeMouse] < MAX_ICONS)) {
                    if(cabs(icons_x[dragging_index[activeMouse]] - icons_x[0]) < 8) {
                    if(cabs(icons_y[dragging_index[activeMouse]] - icons_y[0]) < 8) {
                        icons_f[dragging_index[activeMouse]] = 0;
                    }
                }   
            }
            dragging_index[activeMouse] = 255;
        }
        
        if(dragging_index[activeMouse] < MAX_ICONS) {
            icons_x[dragging_index[activeMouse]] = drag_rel_x[activeMouse] + mouse_display_x[activeMouse];
            icons_y[dragging_index[activeMouse]] = drag_rel_y[activeMouse] + mouse_display_y[activeMouse];
        } else if(dragging_index[activeMouse] == 'w') {
            window_x[dragged_app[activeMouse]] = drag_rel_x[activeMouse] + mouse_display_x[activeMouse];
            window_y[dragged_app[activeMouse]] = drag_rel_y[activeMouse] + mouse_display_y[activeMouse];
        }

        if(frames_since_click[activeMouse] < 255) ++frames_since_click[activeMouse];
        oldMouseStatus[activeMouse] = mouseStatus[activeMouse];
    }

    for(app_draw_index = MAX_APPS-1; app_draw_index != 255; --app_draw_index) {
        if(window_draw_order[app_draw_index] == 0xFF) continue;
        current_app = window_draw_order[app_draw_index];
        if(window_open[current_app]) {
            window_handler[current_app](WINDOW_EVENT_TICK);
        }
    }
}

void desktop_early_draw() {
    queue_draw_sprite(0,0,127,127,0,0,bg_sprite);

   for(app_draw_index = MAX_APPS-1; app_draw_index != 255; --app_draw_index) {
        if(window_draw_order[app_draw_index] == 0xFF) continue;
        current_app = window_draw_order[app_draw_index];
        if(window_open[current_app]) {
            window_handler[current_app](WINDOW_EVENT_EARLY_TICK);
        }
    }
}

void desktop_draw() {
    for(tmp = 0; tmp < MAX_ICONS; ++tmp) {
        if(icons_f[tmp]) {
            queue_draw_sprite_frame(icons_sprite, icons_x[tmp], icons_y[tmp], icons_f[tmp], 0);
        }
    }

    for(app_draw_index = 0; app_draw_index < MAX_APPS; ++app_draw_index) {
        if(window_draw_order[app_draw_index] == 0xFF) continue;
        current_app = window_draw_order[app_draw_index];
        if(window_open[current_app]) {
            window_handler[current_app](WINDOW_EVENT_DRAW);
        }
    }

    for(tmp = 0; tmp < ps2_device_count; ++tmp) {
        if(ps2_device_types[tmp] == PS2_DEVICE_TYPE_BASIC_MOUSE) {
            queue_draw_sprite_frame(icons_sprite, mouse_display_x[tmp], mouse_display_y[tmp], tmp&3, 0);
        }
    }
    queue_clear_border(0);
    await_draw_queue();
}

void desktop_late_update() {
    for(app_draw_index = MAX_APPS-1; app_draw_index != 255; --app_draw_index) {
        if(window_draw_order[app_draw_index] == 0xFF) continue;
        current_app = window_draw_order[app_draw_index];
        if(window_open[current_app]) {
            window_handler[current_app](WINDOW_EVENT_LATE_TICK);
        }
    }

    switch(app_to_launch) {
        case 0: break;
        case 1: draw_app_launch(); break;
        case 2: spike_app_launch(); break;
        case 3: mines_app_launch(); break;
        case 4: mplayer_app_launch(); break;
        case 5: text_edit_app_launch(); break;
    }
    app_to_launch = 0;

    //Flush keyboard buffer after all app handlers done for frame
    while(pop_keyboard().src != 255) {}
}