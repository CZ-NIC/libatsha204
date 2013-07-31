#ifndef LIBATSHA204_H
#define LIBATSHA204_H

#include <stdlib.h>
#include <stdint.h>

/**********************************************************************
 ******** THIS FILE REPRESENTS PUBLIC INTERFACE OF LIBATSHA204 *********
***********************************************************************/

//Internal structure
struct atsha_handle;

//Structure for data exchange
typedef struct {
	size_t bytes;
	unsigned char *data;
} atsha_big_int;

//Library settings and initialization
void atsha_set_verbose();
void atsha_set_log_callback(void (*clb)(const char* msg));
struct atsha_handle *atsha_open_usb_dev(char *path);
struct atsha_handle *atsha_open_emulation(char *serial_number, char *key);
void atsha_close(struct atsha_handle *handle);

//Operations over device
int atsha_dev_rev(struct atsha_handle *handle, uint32_t *revision);
int atsha_random(struct atsha_handle *handle, atsha_big_int *number);
int atsha_slot_read(struct atsha_handle *handle, unsigned char slot_number, atsha_big_int *number);
int atsha_slot_write(struct atsha_handle *handle, unsigned char slot_number, atsha_big_int number);
int atsha_slot_conf_read(struct atsha_handle *handle, unsigned char slot_number, uint16_t *config_word);
int atsha_challenge_response(struct atsha_handle *handle, unsigned char slot_number, atsha_big_int challenge, atsha_big_int *response);
int atsha_challenge_response_mac(struct atsha_handle *handle, unsigned char slot_number, atsha_big_int challenge, atsha_big_int *response);

//Error management
static const int ATSHA_ERR_OK = 0;
static const int ATSHA_ERR_MEMORY_ALLOCATION_ERROR = 1;
static const int ATSHA_ERR_INVALID_INPUT = 2;
static const int ATSHA_ERR_COMMUNICATION = 3;
static const int ATSHA_ERR_BAD_COMMUNICATION_STATUS = 4;
static const int ATSHA_ERR_WAKE_NOT_CONFIRMED = 5;
static const int ATSHA_ERR_NOT_IMPLEMENTED = 6;
static const int ATSHA_ERR_USBCMD_NOT_CONFIRMED = 7;
const char *atsha_error_name(int err);

#endif //LIBATSHA204_H
