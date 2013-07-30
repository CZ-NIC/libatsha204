#ifndef LIBATSHA204_H
#define LIBATSHA204_H

#include <stdlib.h>
#include <stdint.h>

/**********************************************************************
 ******** THIS FILE REPRESENTS PUBLIC INTERFACE OF LIBATSHA204 *********
***********************************************************************/

typedef struct {
	size_t bytes;
	unsigned char *data;
} atsha_big_int;

//Library settings
void atsha_set_verbose();
void atsha_set_log_callback(void (*clb)(const char* msg));
void atsha_emulation_on();
void atsha_emulation_off();

//Operations over device
int atsha_dev_rev(uint32_t *revision);
int atsha_random(atsha_big_int *number);
int atsha_slot_read(unsigned char slot_number, atsha_big_int *number);
int atsha_slot_write(unsigned char slot_number, atsha_big_int number);
int atsha_slot_conf_read(unsigned char slot_number, uint16_t *config_word);

//Error management
static const int ATSHA_ERR_OK = 0;
static const int ATSHA_ERR_MEMORY_ALLOCATION_ERROR = 1;
static const int ATSHA_ERR_COMMUNICATION = 2;
static const int ATSHA_ERR_BAD_COMMUNICATION_STATUS = 3;
static const int ATSHA_ERR_WAKE_NOT_CONFIRMED = 4;
static const int ATSHA_ERR_NOT_IMPLEMENTED = 5;
static const int ATSHA_ERR_USBCMD_NOT_CONFIRMED = 6;
const char *atsha_error_name(int err);

#endif //LIBATSHA204_H
