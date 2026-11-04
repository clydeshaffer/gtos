#ifndef CH37X_USB_H
#define CH37X_USB_H

#define CH375_CMD_GET_IC_VER    0x01
#define CH375_CMD_SET_BAUDRATE  0x02
#define CH375_CMD_ENTER_SLEEP   0x03
#define CH375_CMD_RESET_ALL     0x05
#define CH375_CMD_CHECK_EXIST   0x06
#define CH375_CMD_GET_MAX_LUN   0x0A
#define CH375_CMD_SET_DISK_LUN  0x0B
#define CH375_CMD_SET_PKT_P_SEC 0x0B
#define CH375_CMD_SET_USB_MODE  0x15
#define CH375_CMD_TEST_CONNECT  0x16
#define CH375_CMD_ABORT_NAK     0x17
#define CH375_CMD_GET_STATUS    0x22
#define CH375_CMD_RD_USB_DATA   0x28
#define CH375_CMD_WR_USB_DATA7  0x2B
#define CH375_CMD_DISK_INIT     0x51
#define CH375_CMD_DISK_SIZE     0x53
#define CH375_CMD_DISK_READ     0x54
#define CH375_CMD_DISK_RD_GO    0x55
#define CH375_CMD_DISK_WRITE    0x56
#define CH375_CMD_DISK_WR_GO    0x57
#define CH375_CMD_DISK_INQUIRY  0x58
#define CH375_CMD_DISK_READY    0x59
#define CH375_CMD_DISK_R_SENSE  0x5A

#define CH376_CMD_GET_IC_VER    0x01
#define CH376_CMD_SET_BAUDRATE  0x02
#define CH376_CMD_ENTER_SLEEP   0x03
#define CH376_CMD_RESET_ALL     0x05
#define CH376_CMD_CHECK_EXIST   0x06
#define CH376_CMD_SET_SDO_INT   0x0B
#define CH376_CMD_GET_FILE_SIZE 0x0C 
#define CH376_CMD_SET_USB_MODE  0x15
#define CH376_CMD_GET_STATUS    0x22
#define CH376_CMD_RD_USB_DATA0  0x27
#define CH376_CMD_WR_HOST_DATA  0x2C
#define CH376_CMD_WR_REQ_DATA   0x2D
#define CH376_CMD_WR_OFS_DATA   0x2E
#define CH376_CMD_SET_FILE_NAME 0x2F
#define CH376_CMD_DISK_CONNECT  0x30
#define CH376_CMD_DISK_MOUNT    0x31
#define CH376_CMD_FILE_OPEN     0x32
#define CH376_CMD_FILE_ENUM_GO  0x33
#define CH376_CMD_FILE_CREATE   0x34
#define CH376_CMD_FILE_ERASE    0x35
#define CH376_CMD_FILE_CLOSE    0x36
#define CH376_CMD_DIR_INFO_READ 0x37
#define CH376_CMD_DIR_INFO_SAVE 0x38
#define CH376_CMD_BYTE_LOCATE   0x39
#define CH376_CMD_BYTE_READ     0x3A
#define CH376_CMD_BYTE_RD_GO    0x3B
#define CH376_CMD_BYTE_WRITE    0x3C
#define CH376_CMD_BYTE_WR_GO    0x3D
#define CH376_CMD_DISK_CAPACITY 0x3E
#define CH376_CMD_DISK_QUERY    0x3F
#define CH376_CMD_DIR_CREATE    0x40
#define CH376_CMD_SEC_LOCATE    0x4A
#define CH376_CMD_SEC_READ      0x4B
#define CH376_CMD_SEC_WRITE     0x4C
#define CH376_CMD_DISK_BOC_CMD  0x50
#define CH376_CMD_DISK_READ     0x54
#define CH376_CMD_DISK_RD_GO    0x55
#define CH376_CMD_DISK_WRITE    0x56
#define CH376_CMD_DISK_WR_GO    0x57

#define CH37X_RET_SUCCESS 0x51
#define CH37X_RET_ABORT   0x5F

#define CH376_CARD_ID 0x76

#define CH37X_USB_INT_SUCCESS 0x14
#define CH37X_USB_INT_CONNECT 0x15
#define CH37X_USB_INT_DISCONNECT 0x16
#define CH37X_USB_INT_USB_READY 0x18
#define CH37X_USB_INT_DISK_READ 0x1D
#define CH37X_USB_INT_DISK_WRITE 0x1E
#define CH37X_USB_INT_DISK_ERR 0x1F

#define CH37X_ERR_OPEN_DIR 0x41

extern char ch37x_last_cmd;
extern char ch37x_ready;
extern char ch37x_init_called;
extern char ch37x_debug;
extern char ch37x_plugged_in_device;
extern char ch37x_read_buffer[64];
extern char ch37x_read_buffer_count;
extern char ch37x_read_event_index;
extern char ch37x_last_int_status;

char ch376_init();

void ch376_poll(char exbus_addr);

void ch37x_select(char exbus_addr);

void ch37x_write_cmd_impl();

#define ch37x_write_cmd(c) ch37x_last_cmd=(c); ch37x_write_cmd_impl();

void ch37x_write_data(char cmd);

char ch37x_read_status();

char ch37x_read_data();

char ch37x_poll_for_int();

#endif