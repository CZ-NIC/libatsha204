#ifndef LIBATSHA204_H
#define LIBATSHA204_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/**********************************************************************
 ******** THIS FILE REPRESENTS PUBLIC INTERFACE OF LIBATSHA204 *********
***********************************************************************/

//Internal structure
struct atsha_handle;

//Structure for data exchange
#define ATSHA_MAX_DATA_SIZE 32
typedef struct {
	size_t bytes;
	unsigned char data[ATSHA_MAX_DATA_SIZE];
} atsha_big_int;

//Library settings and initialization
void atsha_set_verbose();
void atsha_set_log_callback(void (*clb)(const char* msg));
struct atsha_handle *atsha_open();
struct atsha_handle *atsha_open_usb_dev(char *path);
struct atsha_handle *atsha_open_i2c_dev();
struct atsha_handle *atsha_open_emulation(char *path);
struct atsha_handle *atsha_open_server_emulation(unsigned char *serial_number, unsigned char *key);
void atsha_close(struct atsha_handle *handle);

//Special
unsigned char atsha_find_slot_number(struct atsha_handle *handle);

//Operations over device
int atsha_dev_rev(struct atsha_handle *handle, uint32_t *revision);
int atsha_random(struct atsha_handle *handle, atsha_big_int *number);
int atsha_slot_read(struct atsha_handle *handle, atsha_big_int *number);
int atsha_low_slot_read(struct atsha_handle *handle, unsigned char slot_number, atsha_big_int *number);
int atsha_slot_write(struct atsha_handle *handle, atsha_big_int number);
int atsha_low_slot_write(struct atsha_handle *handle, unsigned char slot_number, atsha_big_int number);
//int atsha_slot_conf_read(struct atsha_handle *handle, unsigned char slot_number, uint16_t *config_word);
int atsha_challenge_response(struct atsha_handle *handle, atsha_big_int challenge, atsha_big_int *response);
int atsha_low_challenge_response(struct atsha_handle *handle, unsigned char slot_number, atsha_big_int challenge, atsha_big_int *response, bool use_sn_in_digest);
int atsha_challenge_response_mac(struct atsha_handle *handle, atsha_big_int challenge, atsha_big_int *response);
int atsha_low_challenge_response_mac(struct atsha_handle *handle, unsigned char slot_number, atsha_big_int challenge, atsha_big_int *response, bool use_sn_in_digest);
int atsha_serial_number(struct atsha_handle *handle, atsha_big_int *number);
int atsha_raw_conf_read(struct atsha_handle *handle, unsigned char address, atsha_big_int *data);
int atsha_raw_conf_write(struct atsha_handle *handle, unsigned char address, atsha_big_int data);
int atsha_raw_otp_read(struct atsha_handle *handle, unsigned char address, atsha_big_int *data);
int atsha_raw_otp_write(struct atsha_handle *handle, unsigned char address, atsha_big_int data);

//Error management
#define ATSHA_ERR_OK 0
#define ATSHA_ERR_MEMORY_ALLOCATION_ERROR 1
#define ATSHA_ERR_INVALID_INPUT 2
#define ATSHA_ERR_COMMUNICATION 3
#define ATSHA_ERR_BAD_COMMUNICATION_STATUS 4
#define ATSHA_ERR_WAKE_NOT_CONFIRMED 5
#define ATSHA_ERR_NOT_IMPLEMENTED 6
#define ATSHA_ERR_CONFIG_FILE_BAD_FORMAT 7
#define ATSHA_ERR_DNS_GET_KEY 8
#define ATSHA_ERR_USBCMD_NOT_CONFIRMED 9
const char *atsha_error_name(int err);

#endif //LIBATSHA204_H
