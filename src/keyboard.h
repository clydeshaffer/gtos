typedef struct keyboard_packet {
    char data;
    char src;
} keyboard_packet;

void handle_led_keys(unsigned char lastRead, char index);

char keyboard_buf_count();
keyboard_packet pop_keyboard();

char poll_keyboard(char index);