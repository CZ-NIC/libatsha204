#ifndef API_H
#define API_H

#include<stdio.h>
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
	int lockfile;
	struct mpsse_context *i2c;
	unsigned char *sn;
	unsigned char *key;
	uint32_t key_origin;
	unsigned char slot_id;
	unsigned char nonce[32];
};

#define BOTTOM_LAYER_EMULATION 0
#define BOTTOM_LAYER_I2C 1
#define BOTTOM_LAYER_USB 2
#define DNS_ERR_CONST 255

void log_message(const char* msg);
#endif //MAIN_H
