extern char usb_storage_device;

void fs_create_sync(char* name);

char fs_write_data(char* data, char bank, unsigned int length, char stride, int gap);

char fs_close_sync();

char fs_validate_filename_no_ext(char* name);