#ifndef MAIN_H
#define MAIN_H

#include<stdint.h>

/**
 * Struct for global configuration of library
 */
typedef struct {
	int device_fd;
	bool verbose;
	void (*log_callback)(const char* msg);
	int bottom_layer;
} atsha_configuration;

#define BOTTOM_LAYER_EMULATION 0
#define BOTTOM_LAYER_I2C 1
#define BOTTOM_LAYER_USB 2
#define BOTTOM_LAYER_DEFAULT BOTTOM_LAYER_USB

void log_message(const char* msg);
#endif //MAIN_H
