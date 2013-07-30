#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> //close()
#include<sys/file.h> //open()
#include<fcntl.h>
#include<string.h>
#include<stdint.h>
#include<stdbool.h>
#include<assert.h>

#include "atsha204.h"
#include "tools.h"
#include "configuration.h"
#include "atsha204consts.h"
#include "layer_usb.h"
#include "api.h"

extern atsha_configuration g_config;

int wake(struct atsha_handle *handle) {
	int status;
	int tries = TRY_SEND_RECV_ON_COMM_ERROR + 1; //+1 will be eliminated after first iteration
	unsigned char *answer = NULL;

	while (tries >= 0) {
		tries--;
////////////////////////////////////////////////////////////////////////
		switch (handle->bottom_layer) {
			case BOTTOM_LAYER_EMULATION:
				status = usb_wake(handle->fd, &answer);
				break;
			case BOTTOM_LAYER_I2C:
				status = ATSHA_ERR_NOT_IMPLEMENTED;
				break;
			case BOTTOM_LAYER_USB:
				status = usb_wake(handle->fd, &answer);
				break;
		}
////////////////////////////////////////////////////////////////////////
		if (status == ATSHA_ERR_OK) {
			//Check packet consistency and check wake confirmation
			bool packet_ok = check_packet(answer);
			if (!packet_ok || (answer[1] != ATSHA204_STATUS_WAKE_OK)) {
				free(answer);
				answer = NULL;
				if (!packet_ok && g_config.verbose) log_message("ERR: Wake: CRC doesn't match.");
				status = ATSHA_ERR_COMMUNICATION;
				usleep(TRY_SEND_RECV_ON_COMM_ERROR_TOUT);
				continue;
			}

			break;
		} else {
			usleep(TRY_SEND_RECV_ON_COMM_ERROR_TOUT);
		}
	}

	free(answer);
	return status;
}

int idle(struct atsha_handle *handle) {
	int status;
	int tries = TRY_SEND_RECV_ON_COMM_ERROR;

	while (true) {
////////////////////////////////////////////////////////////////////////
		switch (handle->bottom_layer) {
			case BOTTOM_LAYER_EMULATION:
				status = usb_idle(handle->fd);
				break;
			case BOTTOM_LAYER_I2C:
				status = ATSHA_ERR_NOT_IMPLEMENTED;
				break;
			case BOTTOM_LAYER_USB:
				status = usb_idle(handle->fd);
				break;
		}
////////////////////////////////////////////////////////////////////////
		if (status == ATSHA_ERR_OK) return status;
		if (tries < 0) return status;
	}
}

int command(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer) {
	int status;
	int tries = TRY_SEND_RECV_ON_COMM_ERROR + 1; //+1 will be eliminated after first iteration

	while (tries >= 0) {
		tries--;
////////////////////////////////////////////////////////////////////////
		switch (handle->bottom_layer) {
			case BOTTOM_LAYER_EMULATION:
				status = usb_command(handle->fd, raw_packet, answer);
				break;
			case BOTTOM_LAYER_I2C:
				status = ATSHA_ERR_NOT_IMPLEMENTED;
				break;
			case BOTTOM_LAYER_USB:
				status = usb_command(handle->fd, raw_packet, answer);
				break;
		}
////////////////////////////////////////////////////////////////////////
		if (status == ATSHA_ERR_OK) {
			//Check packet consistency and status code
			if (!check_packet(*answer)) {
				free(*answer);
				*answer = NULL;
				if (g_config.verbose) log_message("ERR: Command: CRC doesn't match.");
				status = ATSHA_ERR_COMMUNICATION;
				usleep(TRY_SEND_RECV_ON_COMM_ERROR_TOUT);
				continue;
			}

			if ((*answer)[0] == 4) { //Messages with length 4 are always status codes
				unsigned char atsha204_status = (*answer)[1];
				bool go_trough = true;
				if (atsha204_status == ATSHA204_STATUS_PARSE_ERROR) {
					if (g_config.verbose) log_message("ERR: Bad status: Parse error.");
					go_trough = false;
				} else if (atsha204_status == ATSHA204_STATUS_EXEC_ERROR) {
					if (g_config.verbose) log_message("ERR: Bad status: Execution error.");
					go_trough = false;
				} else if (atsha204_status == ATSHA204_STATUS_COMMUNICATION_ERROR) {
					if (g_config.verbose) log_message("ERR: Bad status: Communication error.");
					go_trough = false;
				} //The rest of status codes are distributed

				if (!go_trough) {
					free(*answer);
					*answer = NULL;
					status = ATSHA_ERR_BAD_COMMUNICATION_STATUS;
					usleep(TRY_SEND_RECV_ON_COMM_ERROR_TOUT);
					continue;
				}
			}

			break;
		} else {
			usleep(TRY_SEND_RECV_ON_COMM_ERROR_TOUT);
		}
	}

	return status;
}
