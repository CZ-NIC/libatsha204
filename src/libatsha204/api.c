#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> //close()
#include<sys/file.h> //open()
#include<fcntl.h>
#include<errno.h>
#include<string.h>
#include<stdint.h>
#include<stdbool.h>

#include "atsha204.h"
#include "api.h"
#include "communication.h"
#include "tools.h"
#include "operations.h"

/**
 * Global variable with configuration and some initial config values.
 */
atsha_configuration g_config = {
	.device_fd = 0,
	.verbose = false,
	.log_callback = NULL
};

static const char *WARNING_WAKE_NOT_CONFIRMED = "Device is possibly still awake";

void log_message(const char* msg) {
	if (g_config.log_callback != NULL) {
		g_config.log_callback(msg);
	}
}

void atsha_set_verbose() {
	g_config.verbose = true;
}

void atsha_set_log_callback(void (*clb)(const char* msg)) {
	g_config.log_callback = clb;
}

struct atsha_handle *atsha_open_usb_dev(char *path) {
	if (path == NULL) return NULL;

	int try_fd = open(path, O_RDWR);
	if (try_fd == -1) {
		log_message("Couldn't open devidce.");
		return NULL;
	}

	struct atsha_handle *handle = (struct atsha_handle *)calloc(1, sizeof(struct atsha_handle));
	if (handle == NULL) return NULL;

	handle->bottom_layer = BOTTOM_LAYER_USB;
	handle->fd = try_fd;
	handle->sn = NULL;
	handle->key = NULL;

	return handle;
}

struct atsha_handle *atsha_open_emulation(char *serial_number, char *key) {
	if (serial_number == NULL || key == NULL) return NULL;

	struct atsha_handle *handle = (struct atsha_handle *)calloc(1, sizeof(struct atsha_handle));
	if (handle == NULL) return NULL;

	handle->bottom_layer = BOTTOM_LAYER_EMULATION;
	handle->sn = serial_number;
	handle->key = key;

	return handle;
}

void atsha_close(struct atsha_handle *handle) {
	if (handle == NULL) return;

	if (handle->bottom_layer == BOTTOM_LAYER_USB) {
		close(handle->bottom_layer);
	}
	free(handle->sn);
	free(handle->key);

	free(handle);
}

int atsha_dev_rev(struct atsha_handle *handle, uint32_t *revision) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_dev_rev();
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	*revision = op_dev_rev_recv(answer);

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_random(struct atsha_handle *handle, atsha_big_int *number) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_random();
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	number->bytes = op_random_recv(answer, &(number->data));
	if (number->bytes == 0) {
		free(packet);
		free(answer);
		return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;
	}

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_slot_read(struct atsha_handle *handle, unsigned char slot_number, atsha_big_int *number) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_raw_read(get_zone_config(IO_MEM_DATA, IO_RW_NON_ENC, IO_RW_32_BYTES), get_slot_address(slot_number));
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	number->bytes = op_raw_read_recv(answer, &(number->data));
	if (number->bytes == 0) {
		free(packet);
		free(answer);
		return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;
	}

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_slot_write(struct atsha_handle *handle, unsigned char slot_number, atsha_big_int number) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_raw_write(get_zone_config(IO_MEM_DATA, IO_RW_NON_ENC, IO_RW_32_BYTES), get_slot_address(slot_number), number.bytes, number.data);
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	status = op_raw_write_recv(answer);
	if (status != ATSHA_ERR_OK) {
		return status;
	}

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}

int atsha_slot_conf_read(struct atsha_handle *handle, unsigned char slot_number, uint16_t *config_word) {
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;
	atsha_big_int number;

	//Wakeup device
	status = wake(handle);
	if (status != ATSHA_ERR_OK) return status;

	packet = op_raw_read(get_zone_config(IO_MEM_CONFIG, IO_RW_NON_ENC, IO_RW_4_BYTES), get_slot_config_address(slot_number));
	if (!packet) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	status = command(handle, packet, &answer);
	if (status != ATSHA_ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	number.bytes = op_raw_read_recv(answer, &(number.data));
	if (number.bytes == 0) {
		free(packet);
		free(answer);
		return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;
	}

	*config_word = get_slot_config_word(slot_number, number.data);
	free(number.data);

	//Let device sleep
	status = idle(handle);
	if (status != ATSHA_ERR_OK) {
		log_message(WARNING_WAKE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

	return ATSHA_ERR_OK;
}