#include "gametank.h"
#include "exbus.h"

void exbus_select_device(char addr) {
    EXBUS_MODE = EXBUS_SELECT;
    via[DDRB] = 0xFF;
    via[ORB] = addr;
    EXBUS_MODE = EXBUS_IGNORE;
    via[DDRB] = 0;
}

char exbus_query_device(char addr) {
    EXBUS_MODE = EXBUS_SELECT;
    via[DDRB] = 0xFF;
    via[ORB] = addr;
    EXBUS_MODE = EXBUS_IGNORE;
    via[ORB] = 0;
    via[DDRB] = 0;
    EXBUS_MODE = EXBUS_REPORT;
    return via[ORB];
}