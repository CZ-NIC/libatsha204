#ifndef ERROR_H
#define ERROR_H

static const int ERR_OK = 0;
static const int ERR_CRC = 1;
static const int ERR_SEND = 2;
static const int ERR_READ = 3;
static const int ERR_WAKE_NOT_CONFIRMED = 4;
static const int ERR_ALLOCATION = 5;
static const int ERR_USBCMD_NOT_CONFIRMED = 6;

const char *error_name(int err);

#endif //ERROR_H


