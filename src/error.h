#ifndef ERROR_H
#define ERROR_H

static const int ATSHA_ERR_OK = 0;
static const int ATSHA_ERR_MEMORY_ALLOCATION_ERROR = 1;
static const int ATSHA_ERR_COMMUNICATION = 2;
static const int ATSHA_ERR_WAKE_NOT_CONFIRMED = 3;

static const int ATSHA_ERR_USBCMD_NOT_CONFIRMED = 6;

const char *atsha_error_name(int err);

#endif //ERROR_H


