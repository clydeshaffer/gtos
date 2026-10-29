#define CB2HI 0b11100000
#define CB2LO 0b11000000
#define CA2HI 0b00001110
#define CA2LO 0b00001100

#define EXBUS_DEVICE (CA2LO | CB2LO)
#define EXBUS_REPORT (CA2HI | CB2LO)
#define EXBUS_IGNORE (CA2LO | CB2HI)
#define EXBUS_SELECT (CA2HI | CB2HI)
#define EXBUS_MODE via[0x0C]

void exbus_select_device(char addr);

char exbus_query_device(char addr);