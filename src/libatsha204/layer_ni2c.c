#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<stdbool.h>
#include<unistd.h>
#include <errno.h>
//#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "atsha204.h"
#include "atsha204consts.h"
#include "configuration.h"
#include "communication.h"
#include "tools.h"
#include "api.h"

extern atsha_configuration g_config;

void ni2c_wait() {
	usleep(ATSHA204_I2C_CMD_TOUT);
}

static int ni2c_read(struct atsha_handle *handle, unsigned char **answer) {
	unsigned char data[BUFFSIZE_NI2C];

	if (read(handle->fd, data, BUFFSIZE_NI2C) < 0) {
		return ATSHA_ERR_COMMUNICATION;
	}

	*answer = calloc(data[0], sizeof(char));
	if (*answer == NULL) return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;

	memcpy(*answer, data, data[0]);

	return ATSHA_ERR_OK;
}

int ni2c_wake(struct atsha_handle *handle, unsigned char **answer) {
	unsigned char wr_wake[] = { 0x00 };
	int status;

	//OK, I know, this is weird. But we really need to not check error status
	write(handle->fd, wr_wake, 1);

	ni2c_wait();

	status = ni2c_read(handle, answer);
	if (status != ATSHA_ERR_OK) {
		return status;
	}

	return ATSHA_ERR_OK;
}

int ni2c_idle(struct atsha_handle *handle) {
	unsigned char wr_idle[] = { ATSHA204_I2C_WA_IDLE };

	write(handle->fd, wr_idle, 1);

	return ATSHA_ERR_OK;
}

int ni2c_command(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer) {
	unsigned char wr_cmd[] = { ATSHA204_I2C_WA_COMMAND };
	int status;
//TODO: SEND 0x03
	if (write(handle->fd, raw_packet, raw_packet[0]) < 0) {
		return ATSHA_ERR_COMMUNICATION;
	}

	ni2c_wait();

	status = ni2c_read(handle, answer);
	if (status != ATSHA_ERR_OK) {
		return status;
	}

	return ATSHA_ERR_OK;
}
