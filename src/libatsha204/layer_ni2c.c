/*
 * libatsha204 is small library and set of tools for Amel ATSHA204 crypto chip
 *
 * Copyright (C) 2013 CZ.NIC, z.s.p.o. (http://www.nic.cz/)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
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
		log_message("layer_ni2c: ni2c_read: Read packet failed");
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
	unsigned char *send_buffer = (unsigned char *) calloc(raw_packet[0] + 1, sizeof(unsigned char));
	if (send_buffer == NULL) {
		log_message("layer_ni2c: ni2c_command: Send buffer memory allocation error");
		return ATSHA_ERR_MEMORY_ALLOCATION_ERROR;
	}
	send_buffer[0] = ATSHA204_I2C_WA_COMMAND;
	memcpy((send_buffer + 1), raw_packet, raw_packet[0]);

	if (write(handle->fd, send_buffer, raw_packet[0] + 1) < 0) {
		free(send_buffer);
		log_message("layer_ni2c: ni2c_command: Send command packet");
		return ATSHA_ERR_COMMUNICATION;
	}

	free(send_buffer);

	ni2c_wait();

	int status;
	status = ni2c_read(handle, answer);
	if (status != ATSHA_ERR_OK) {
		return status;
	}

	return ATSHA_ERR_OK;
}
