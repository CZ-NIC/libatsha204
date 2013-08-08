#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> //close()
#include<sys/file.h> //open()
#include<fcntl.h>
#include<errno.h>
#include<string.h>
#include<stdint.h>
#include<stdbool.h>

#include "configuration.h"
#include "atsha204consts.h"
#include "atsha204.h"
#include "api.h"

/**
 * Global variable with configuration and some initial config values.
 */
atsha_configuration g_config;

unsigned char atsha_find_slot_number(struct atsha_handle *handle) {
	if (handle->is_srv_emulation == true) {
		/**
		 * It is not important what is the returned value.
		 * This line is just saving server resources
		 */
		return 0;
	}

	//OK, here will be some difficult DNS magic
	//For now is one hardcoded open and free to use slot good enough
	return 8;

	return DNS_ERR_CONST;
}
