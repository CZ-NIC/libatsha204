#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<stdbool.h>
#include<unistd.h>
#include <mpsse.h>

#include "atsha204.h"
#include "atsha204consts.h"
#include "configuration.h"
#include "communication.h"
#include "tools.h"
#include "api.h"

extern atsha_configuration g_config;

static void i2c_wait() {
	usleep(ATSHA204_I2C_CMD_TOUT);
}

static int i2c_read(struct atsha_handle *handle, unsigned char **answer) {
	unsigned char wr_addr[] = { ATSHA204_I2C_ADDRESS };
	unsigned char *data;
	int status;

	wr_addr[0] |= 1;

	status = Start(handle->i2c);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_read: Start");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = Write(handle->i2c, (char *)wr_addr, 1);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_read: Write addr");
		return ATSHA_ERR_COMMUNICATION;
	}

	data = (unsigned char *)Read(handle->i2c, BUFFSIZE_I2C);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_read: Read");
		return ATSHA_ERR_COMMUNICATION;
	}

	if (data == NULL) {
		log_message("layer_i2c: i2c_command: No data read");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = Stop(handle->i2c);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_read: Stop");
		return ATSHA_ERR_COMMUNICATION;
	}
print_buffer_content(data, data[0]);

	*answer = calloc(data[0], sizeof(char));
	if (*answer == NULL) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	memcpy(*answer, data, data[0]);
	free(data);

	return ATSHA_ERR_OK;
}

int i2c_wake(struct atsha_handle *handle, unsigned char **answer) {
	char wr_wake[] = { 0x00 };
	int status;

log_message("WAKE");
	status = SetClock(handle->i2c, ATSHA204_I2C_WAKE_CLOCK);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_wake: SetClock - set frequency for wake");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = Start(handle->i2c);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_wake: Start");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = Write(handle->i2c, wr_wake, 1);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_wake: Write");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = Stop(handle->i2c);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_wake: Stop");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = SetClock(handle->i2c, FOUR_HUNDRED_KHZ);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_wake: SetClock - set working frequency");
		return ATSHA_ERR_COMMUNICATION;
	}

	i2c_wait();

	status = i2c_read(handle, answer);
	if (status != ATSHA_ERR_OK) {
		return status;
	}

	return ATSHA_ERR_OK;
}

int i2c_command(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer) {
	unsigned char wr_addr[] = { ATSHA204_I2C_ADDRESS };
	int status;

log_message("COMMAND");
	status = Start(handle->i2c);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_command: Start");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = Write(handle->i2c, (char *)wr_addr, 1);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_command: Write addr");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = Write(handle->i2c, (char *)raw_packet, raw_packet[0]);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_command: Write packet");
		return ATSHA_ERR_COMMUNICATION;
	}

	status = Stop(handle->i2c);
	if (status != MPSSE_OK) {
		log_message("layer_i2c: i2c_command: Stop");
		return ATSHA_ERR_COMMUNICATION;
	}

	i2c_wait();

	status = i2c_read(handle, answer);
	if (status != ATSHA_ERR_OK) {
		return status;
	}

	return ATSHA_ERR_OK;
}
