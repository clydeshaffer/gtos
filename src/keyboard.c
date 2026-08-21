#include "ps2.h"
#include "keyboard.h"

char ledMask[MAX_PS2_CARDS] = {0};
char oldLedMask[MAX_PS2_CARDS] = {0};
static char tmp;

//Assumed to be power of two
#define KEY_BUF_MAX 64

char keystroke_count=0;
char keystroke_head=0;
char keystroke_tail=0;
char keystroke_src[KEY_BUF_MAX];
char keystroke_buf[KEY_BUF_MAX];
static char isBreak[MAX_PS2_CARDS] = {0};

void handle_led_keys(unsigned char lastRead, char index) {
    if((lastRead & 0xFF) == 0x77) {
        ledMask[index] ^= 2;
    }

    if((lastRead & 0xFF) == 0x58) {
        ledMask[index] ^= 4;
    }

    if((lastRead & 0xFF) == 0x7E) {
        ledMask[index] ^= 1;
    }

    if(ledMask[index] != oldLedMask[index]) {
        oldLedMask[index] = ledMask[index];
        send_byte(0xED);
        wait_for_packet();
        lastRead = read_byte();
        //Serial.println(lastRead & 0xFF, HEX);
        clear_clock_inhibit();
        wait_for_packet();
        clear_clock_inhibit();
        send_byte(ledMask[index]);
        wait_for_packet();
        lastRead = read_byte();
        //Serial.println(lastRead & 0xFF, HEX);
    }
}

char keyboard_buf_count() {
    return keystroke_count;
}

static keyboard_packet tmp_pkt;
keyboard_packet pop_keyboard() {
    if(!keystroke_count) {
        tmp_pkt.src = 255;
        tmp_pkt.data = 0;
        return tmp_pkt;
    }
    --keystroke_count;
    tmp_pkt.data = keystroke_buf[keystroke_tail];
    tmp_pkt.src = keystroke_src[keystroke_tail];
    ++keystroke_tail;
    keystroke_tail &= (KEY_BUF_MAX-1);
    return tmp_pkt;
}

char poll_keyboard(char index) {
    ps2_select(index);
    if(keystroke_count == KEY_BUF_MAX) {
        return 0;
    }
    if(ps2_data_ready()) {
        keystroke_buf[keystroke_head] = read_byte();

        if(keystroke_buf[keystroke_head] == 0xF0) {
            isBreak[index] = 1;
        } else {
            if(!isBreak[index]) {
                handle_led_keys(keystroke_buf[keystroke_head], index);
            }
            isBreak[index] = 0;
        }

        keystroke_src[keystroke_head] = index;
        ++keystroke_head;
        keystroke_head &= (KEY_BUF_MAX-1);
        ++keystroke_count;
        return 1;
    } else {
        return 0;
    }
}
