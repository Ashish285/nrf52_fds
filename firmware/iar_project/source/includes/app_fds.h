#ifndef APP_FDS_H__
#define APP_FDS_H__


void init_fds();

void update_record_in_flash(char *ble_data);

void fds_write();

void fds_read();

void do_fds_gc();

#endif //APP_FDS_H__
