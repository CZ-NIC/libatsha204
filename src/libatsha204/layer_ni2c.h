#ifndef LAYER_NI2C_H
#define LAYER_NI2C_H

#include<stdbool.h>

void ni2c_wait();
int ni2c_wake(struct atsha_handle *handle, unsigned char **answer);
int ni2c_idle(struct atsha_handle *handle);
int ni2c_command(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer);

#endif //LAYER_I2C_H
