#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include "atsha204consts.h"

#define TRY_SEND_RECV_ON_COMM_ERROR 5
#define TRY_SEND_RECV_ON_COMM_ERROR_TOUT 2000000
										//in microseconds (2s)
#define BUFFSIZE_USB 1024
#define BUFFSIZE_I2C ATSHA204_IO_BUFFER
#define BUFFSIZE_DNS 512
#define BUFFSIZE_LINE 128

#ifndef DEFAULT_EMULATION_CONFIG_PATH
#define DEFAULT_EMULATION_CONFIG_PATH "atsha204.sw"
#endif
#define DEFAULT_USB_DEV_PATH "/dev/hidraw0"
#define DEFAULT_USE_SN_IN_DIGEST true
#define DEFAULT_DNS_RECORD_FIND_KEY "use_key.extremehost.cz"

#define USE_LAYER_EMULATION 0
#define USE_LAYER_I2C 1
#define USE_LAYER_USB 2
#ifndef USE_LAYER
#define USE_LAYER USE_LAYER_USB
#endif

#endif //CONFIGURATION_H
