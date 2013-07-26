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
} atsha_configuration;

void log_message(const char* msg);

#endif //MAIN_H
