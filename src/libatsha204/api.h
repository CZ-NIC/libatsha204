#ifndef API_H
#define API_H

#include<stdint.h>

/**
 * Struct for global configuration of library
 */
typedef struct {
	int device_fd;
	bool verbose;
	void (*log_callback)(const char* msg);
} atsha_configuration;

struct atsha_handle {
	int bottom_layer;
	bool is_srv_emulation;
	int fd;
	FILE *file;
	unsigned char *sn;
	unsigned char *key;
};

#define BOTTOM_LAYER_EMULATION 0
#define BOTTOM_LAYER_I2C 1
#define BOTTOM_LAYER_USB 2

void log_message(const char* msg);
#endif //MAIN_H
