//Personal System 2 not Play Station 2
#include "gametank.h"

#define MAX_PS2_CARDS 4
extern char ps2_device_count;
extern char ps2_device_types[MAX_PS2_CARDS];

#define PS2_DEVICE_TYPE_UNKNOWN 0
#define PS2_DEVICE_TYPE_BASIC_KEYBOARD 1
#define PS2_DEVICE_TYPE_BASIC_MOUSE 2

void ps2_select(char index);

void ps2_init();

char read_byte();

void send_byte(char b);

char wait_for_packet();

void clear_clock_inhibit();

int send_byte_and_get_response(unsigned char b, unsigned char* buf, int lim);

char ps2_data_ready();