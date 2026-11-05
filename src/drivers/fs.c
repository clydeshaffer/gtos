#include "fs.h"
#include "ch37x_usb.h"
#include "../gt/banking.h"

#pragma code-name (push, "PROG0")
#pragma rodata-name (push, "PROG0")

const char forbidden_filename_chars[] = "\\/:*?\"<>|"; 

char usb_storage_device;

void fs_create_sync(char* name) {
    char i;
    if(!ch37x_init_called) return;
    ch37x_select(usb_storage_device);
    ch37x_write_cmd(CH376_CMD_SET_FILE_NAME);
    i = 0;
    while(name[i] != 0) {
        if(name[i] != ' ') {
            ch37x_write_data(name[i]);
        }
        i++;
    }
    ch37x_write_data(0);
    ch37x_write_cmd(CH376_CMD_FILE_CREATE);
    while(ch37x_poll_for_int() == 0xFF) {}
}

char fs_close_sync() {
    ch37x_write_cmd(CH376_CMD_FILE_CLOSE);
    ch37x_write_data(1);
    return ch37x_wait_for_int() == CH37X_USB_INT_SUCCESS;
}

char fs_validate_filename_no_ext(char* name) {
    while(*name != 0) {
        if(*name == forbidden_filename_chars[0]) return 0;
        if(*name == forbidden_filename_chars[1]) return 0;
        if(*name == forbidden_filename_chars[2]) return 0;
        if(*name == forbidden_filename_chars[3]) return 0;
        if(*name == forbidden_filename_chars[4]) return 0;
        if(*name == forbidden_filename_chars[5]) return 0;
        if(*name == forbidden_filename_chars[6]) return 0;
        if(*name == forbidden_filename_chars[7]) return 0;
        if(*name == forbidden_filename_chars[8]) return 0;
        if(*name == '.') return 1;
        if(*name == ' ') break;
        name++;
    }
    //check if space is followed by nonspace
    while(*name != 0) {
        if(*name == '.') return 1;
        if(*name != ' ') return 0;
        name++;
    }
    return 1;
}

#pragma code-name(pop)

char fs_write_data(char* data, char bank, unsigned int length, char stride, int gap) {
    char i, st;
    if(!ch37x_init_called) return 0;
    ch37x_select(usb_storage_device);
    ch37x_write_cmd(CH376_CMD_BYTE_WRITE);
    ch37x_write_data(length & 0x00FF);
    ch37x_write_data(length >> 8);
    if(ch37x_wait_for_int() != CH37X_USB_INT_DISK_WRITE) {
        ch37x_write_cmd(CH376_CMD_FILE_CLOSE);
        return 0;
    }
    st = stride;
    push_rom_bank();
    change_rom_bank(bank);
    do {
        ch37x_write_cmd(CH376_CMD_WR_REQ_DATA);
        i = ch37x_read_data();
        while(i--) {
            ch37x_write_data(*(data++));
            --st;
            if(st == 0) {
                data += gap;
                st = stride;
            }
        }
        ch37x_write_cmd(CH376_CMD_BYTE_WR_GO);
    } while(ch37x_wait_for_int() == CH37X_USB_INT_DISK_WRITE);
    pop_rom_bank();
    return ch37x_last_int_status == CH37X_USB_INT_SUCCESS;
}