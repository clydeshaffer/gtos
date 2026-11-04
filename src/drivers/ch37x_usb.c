#include "../gt/gametank.h"
#include "../util.h"
#include "exbus.h"
#include "ch37x_usb.h"

#define WRITE_STROBE (1<<6)
#define READ_STROBE (1<<5)
#define DATA_CMD_BIT (1<<4)
#define CH37X_INT (1<<3)

#define SET(bit) via[ORA] |= (bit)
#define CLEAR(bit) via[ORA] &= ~(bit)

char ch37x_last_cmd = 0;
char ch37x_ready = 0;
char ch37x_init_called = 0;
char ch37x_debug = 0;
char ch37x_plugged_in_device = 0;
char ch37x_read_buffer[64];
char ch37x_read_buffer_count = 0;
char ch37x_read_event_index;
char ch37x_last_int_status;

void ch37x_select(char exbus_addr) {
    exbus_select_device(exbus_addr);
    via[DDRA] |= (WRITE_STROBE | READ_STROBE | DATA_CMD_BIT);
    via[DDRA] &= ~CH37X_INT;
    via[ORA] |= (WRITE_STROBE | READ_STROBE | DATA_CMD_BIT);
    EXBUS_MODE = EXBUS_DEVICE;
}

void ch37x_write_cmd_impl() {
  SET(DATA_CMD_BIT);
  via[DDRB] = 0xFF;
  via[ORB] = ch37x_last_cmd;
  CLEAR(WRITE_STROBE);
  SET(WRITE_STROBE);
  via[DDRB] = 0;
}

void ch37x_write_data(char cmd) {
  CLEAR(DATA_CMD_BIT);
  via[DDRB] = 0xFF;
  via[ORB] = cmd;
  CLEAR(WRITE_STROBE);
  SET(WRITE_STROBE);
  via[DDRB] = 0;
}

char ch37x_read_status() {
  char byte;
  SET(DATA_CMD_BIT);
  via[DDRB] = 0;
  CLEAR(READ_STROBE);
  byte = via[ORB];
  SET(READ_STROBE);
  return byte;
}

char ch37x_read_data() {
  char byte;
  CLEAR(DATA_CMD_BIT);
  via[DDRB] = 0;
  CLEAR(READ_STROBE);
  byte = via[ORB];
  SET(READ_STROBE);
  return byte;
}

char ch37x_poll_for_int() {

    if(~via[ORA] & CH37X_INT) {
        ch37x_write_cmd(CH376_CMD_GET_STATUS);
        return(ch37x_read_data());
    }
    return 0xFF;
}

void ch37x_set_usb_mode(char mode) {
    ch37x_write_cmd(CH375_CMD_SET_USB_MODE);
    ch37x_write_data(mode);
}

char ch376_init() {
    char exbus_addr;
    exbus_addr = 0;
    do {
        if(exbus_query_device(exbus_addr) == CH376_CARD_ID) goto found_ch376;
        ++exbus_addr;
    } while (exbus_addr != 0);
    return 0;
found_ch376:
    ch37x_select(exbus_addr);

    ch37x_write_cmd(CH375_CMD_CHECK_EXIST);
    ch37x_write_data(0x55);
    delayMicroseconds(10);
    ch37x_last_int_status = ch37x_read_data();
    if(ch37x_last_int_status == 0xAA) {
        ch37x_write_cmd(CH375_CMD_RESET_ALL);
        delayMicroseconds(255);
        delayMicroseconds(255);
        ch37x_set_usb_mode(0x05);
        ch37x_init_called = 1;
    }
    return exbus_addr;
}

void ch376_poll(char exbus_addr) {
    char int_status, last_cmd, i;
    if(!ch37x_init_called) return;
    ch37x_select(exbus_addr);
    last_cmd = ch37x_last_cmd;
    int_status = ch37x_poll_for_int();
    if(int_status == 0xFF) return;
    ch37x_last_int_status = int_status;
    switch(int_status) {
        case CH37X_USB_INT_CONNECT:
        ch37x_plugged_in_device = 1;
        ch37x_set_usb_mode(0x07);
        ch37x_set_usb_mode(0x06);
        while(ch37x_poll_for_int() == 0xFF) {}
        ch37x_write_cmd(CH376_CMD_DISK_MOUNT);
        break;
        case CH37X_USB_INT_DISCONNECT:
        ch37x_plugged_in_device = 0;
        ch37x_set_usb_mode(0x05);
        break;
        case CH37X_USB_INT_SUCCESS:
        if(last_cmd == CH376_CMD_DISK_MOUNT) {
            ch37x_plugged_in_device = 2;
        }
        break;
        case CH37X_USB_INT_DISK_READ:
        ++ch37x_read_event_index;
        ch37x_write_cmd(CH376_CMD_RD_USB_DATA0);
        ch37x_read_buffer_count = ch37x_read_data();
        for(i = 0; i < ch37x_read_buffer_count; ++i) {
            ch37x_read_buffer[i] = ch37x_read_data();
        }
        break;
        case CH37X_USB_INT_DISK_WRITE:
        break;
        case CH37X_USB_INT_DISK_ERR:
        if(last_cmd == CH376_CMD_DISK_MOUNT) {
            ch37x_write_cmd(CH376_CMD_DISK_MOUNT);
        }
        break;
        default:
        break;
    }
}