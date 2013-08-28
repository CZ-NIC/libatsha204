#ifndef API_H
#define API_H

#include<stdio.h>
#include<stdint.h>

/**
 * \file api.h
 * \brief Definition of internal structures
 */

/**
 * \brief Global configuration of library
 */
typedef struct {
	bool verbose; ///<Enable verbose mode
	void (*log_callback)(const char* msg); ///<Callback for error reporting
} atsha_configuration;

/**
 * \brief Instance of library
 *
 * There may be several parallel instances
 */
struct atsha_handle {
	int bottom_layer; ///<What kind of bottom layer is used
	bool is_srv_emulation; ///<Server-side or client-side emulation?
	int fd;  ///<File descriptor of binary file (e.g. USB layer file)
	FILE *file; ///<Text file handler, mainly for emulation
	int lockfile; ///<File descriptor of lock file
	struct mpsse_context *i2c; ///<Instance of libmpsse library
	unsigned char *sn; ///<Serial number for server-side emulation and for caching
	unsigned char *key; ///<Key for server-side emulation
	uint32_t key_origin; ///<Cached key origin value
	bool key_origin_cached; ///<It key origin value cached?
	unsigned char slot_id; ///<Cached key origin value that is read from OTP memory
	unsigned char nonce[32]; ///<Emulation of TempKey memory slot
};

#define BOTTOM_LAYER_EMULATION 0
#define BOTTOM_LAYER_I2C 1
#define BOTTOM_LAYER_USB 2
#define DNS_ERR_CONST 255

/**
 * \brief Use callback (from global configuration) and send message through it
 */
void log_message(const char* msg);
#endif //MAIN_H
