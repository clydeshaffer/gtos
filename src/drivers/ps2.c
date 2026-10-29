#include "../gt/gametank.h"
#include "ps2.h"
#include "../util.h"
#include "exbus.h"

#define CLEAR_INHIBIT (1<<5)
#define WRITE_STROBE (1<<4)
#define READ_STROBE (1<<3)
#define PACKET_DONE (1<<6)

#define SET(bit) via[ORA] |= bit
#define CLEAR(bit) via[ORA] &= ~bit

char ps2_device_count;
char ps2_device_types[MAX_PS2_CARDS];
char ps2_exbus_addrs[MAX_PS2_CARDS];

static char tmp;
static char resb;
static char response_buf[8];
static char i;

void ps2_select(char index) {
  exbus_select_device(ps2_exbus_addrs[index]);
  EXBUS_MODE = EXBUS_DEVICE;

  via[DDRA] |= (CLEAR_INHIBIT | WRITE_STROBE | READ_STROBE);
  via[DDRA] &= ~PACKET_DONE;
  via[ORA] |= (CLEAR_INHIBIT | WRITE_STROBE | READ_STROBE);
  via[DDRB] = 0;
}

char ps2_identify_device() {
  resb = send_byte_and_get_response(0xF2, response_buf, 8);
  if((resb == 2) && ((response_buf[1] == 0) || (response_buf[1] == 3) || (response_buf[1] == 4))) {
      return PS2_DEVICE_TYPE_BASIC_MOUSE;
  } else {
      return PS2_DEVICE_TYPE_BASIC_KEYBOARD; //TODO check for actual keyboard types and return UNKNOWN if no match
  }
}

void ps2_reset_device() {
  send_byte_and_get_response(0xFF, response_buf, 8);
}

void ps2_start_scanning() {
  send_byte_and_get_response(0xF4, response_buf, 8);
}

void ps2_stop_scanning() {
  send_byte_and_get_response(0xF5, response_buf, 8);
}

void ps2_init() {
    ps2_device_count = 0;
    for(i = 0; i < 255; ++i) {
        if(exbus_query_device(i) == 0xF2) {
            ps2_exbus_addrs[ps2_device_count] = i;
            ++ps2_device_count;
            if(ps2_device_count == MAX_PS2_CARDS) {
                break;
            }            
        }
    }

    for(i = 0; i < ps2_device_count; ++i) {
      ps2_select(i);
      ps2_reset_device();
      delayMicroseconds(100);
      ps2_stop_scanning();
      delayMicroseconds(100);
      ps2_device_types[i] = ps2_identify_device();
      delayMicroseconds(100);
      ps2_start_scanning();
    }

}

char read_byte() {
    CLEAR(READ_STROBE);
    SET(READ_STROBE);
    CLEAR(READ_STROBE);
    tmp = via[ORB];
    SET(READ_STROBE);
    CLEAR(CLEAR_INHIBIT);
    SET(CLEAR_INHIBIT);
    return tmp;
}

void send_byte(char b) {
    via[ORA] |= PACKET_DONE;
    via[DDRA] |= PACKET_DONE;
    via[DDRB] = 0xFF;
    via[ORB] = b;//scramble(b);
    delayMicroseconds(100);
    CLEAR(WRITE_STROBE);
    via[DDRA] &= ~PACKET_DONE;
    via[ORA] &= ~PACKET_DONE;
    SET(WRITE_STROBE);
    CLEAR(CLEAR_INHIBIT);
    SET(CLEAR_INHIBIT);
    via[DDRB] = 0;
}

char wait_for_packet() {
  int cnt = 0;
  while(!(via[ORA] & PACKET_DONE)) {
    delayMicroseconds(100);
    ++cnt;
    if(cnt > 500) {
      return 0;
    }
  }
  return 1;
}

void clear_clock_inhibit() {
  CLEAR(CLEAR_INHIBIT);
  SET(CLEAR_INHIBIT);
}

int send_byte_and_get_response(unsigned char b, unsigned char* buf, int lim) {
  int response_bytes = 0;

  send_byte(b);
  wait_for_packet();
  tmp = read_byte();
  clear_clock_inhibit();
  while((response_bytes < lim) && wait_for_packet()) {
    tmp = read_byte();
    clear_clock_inhibit();
    buf[response_bytes] = tmp;
    response_bytes++;
  }
  return response_bytes;
}

char ps2_data_ready() {
    return via[ORA] & PACKET_DONE;
}