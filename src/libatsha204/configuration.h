#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "atsha204consts.h"

#define TRY_SEND_RECV_ON_COMM_ERROR 5
#define TRY_SEND_RECV_ON_COMM_ERROR_TOUT 2000000
										//in microseconds (2s)
#define BUFFSIZE_USB 1024
#define BUFFSIZE_I2C ATSHA204_IO_BUFFER
#define BUFFSIZE_NI2C ATSHA204_IO_BUFFER
#define BUFFSIZE_DNS 512
#define BUFFSIZE_LINE 128

#define USE_OUR_SN true

#ifndef DEFAULT_EMULATION_CONFIG_PATH
#define DEFAULT_EMULATION_CONFIG_PATH "atsha204.sw"
#endif
#define DEFAULT_USB_DEV_PATH "/dev/hidraw0"
#define NI2C_DEV_PATH_LOCAL "/dev/i2c-0"
#define NI2C_DEV_PATH_REMOTE "/dev/i2c-1"
#ifdef ENDPOINT_REMOTE
#warning "Je tam REMOTE"
#define DEFAULT_NI2C_DEV_PATH NI2C_DEV_PATH_REMOTE
#else
#define DEFAULT_NI2C_DEV_PATH NI2C_DEV_PATH_LOCAL
#endif
#define DEFAULT_USE_SN_IN_DIGEST true
#define DEFAULT_DNS_RECORD_FIND_KEY "atsha-key.turris.cz"
#define DEFAULT_DNSSEC_ROOT_KEY "/etc/unbound/root.key"
#define LOCK_FILE "/tmp/libatsha204.lock"
#define LOCK_TRY_TOUT 10000
#define LOCK_TRY_MAX 2.2

#define USE_LAYER_EMULATION 0
#define USE_LAYER_NI2C 1
#define USE_LAYER_I2C 2
#define USE_LAYER_USB 3
#ifndef USE_LAYER
#define USE_LAYER USE_LAYER_NI2C
#endif

#endif //CONFIGURATION_H
