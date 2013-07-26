#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#define USE_LAYER_USB
//#define USE_LAYER_I2C
//#define USE_LAYER_EMUL

static const int TRY_SEND_RECV_ON_COMM_ERROR = 5;
static const int TRY_SEND_RECV_ON_COMM_ERROR_TOUT = 2000000; //in microseconds (2s)
static const size_t BUFFSIZE = 1024;


#endif //CONFIGURATION_H
