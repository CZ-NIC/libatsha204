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

#include <unistd.h> //close()
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "atsha204.h"
#include "tools.h"
#include "configuration.h"
#include "atsha204consts.h"
#include "layer_ni2c.h"
#include "emulation.h"
#include "api.h"

extern atsha_configuration g_config;

static void try_send_and_recv_sleep(struct atsha_handle *handle) {
	if (handle->bottom_layer == BOTTOM_LAYER_NI2C) {
		ni2c_wait();
	}
}

int wake(struct atsha_handle *handle) {
	int status;
	int tries = TRY_SEND_RECV_ON_COMM_ERROR + 1; //+1 will be eliminated after first iteration
	unsigned char *answer = NULL;

	if (handle->wake_is_expected) {
		return ATSHA_ERR_OK;
	}

	while (tries >= 0) {
		tries--;

		switch (handle->bottom_layer) {
			case BOTTOM_LAYER_EMULATION:
				handle->wake_is_expected = true;
				return ATSHA_ERR_OK; //Wake is dummy in implementation. Always is successful.
				break;
			case BOTTOM_LAYER_NI2C:
				status = ni2c_wake(handle, &answer);
				break;
		}

		if (status == ATSHA_ERR_OK) {
			//Check packet consistency and check wake confirmation
			bool packet_ok = check_packet(answer);
			if (!packet_ok || (answer[1] != ATSHA204_STATUS_WAKE_OK)) {
				free(answer);
				answer = NULL;
				if (!packet_ok) log_message("communication: wake: CRC doesn't match.");
				status = ATSHA_ERR_COMMUNICATION;
				try_send_and_recv_sleep(handle);
				continue;
			}

			break;
		} else {
			try_send_and_recv_sleep(handle);
		}
	}

	handle->wake_is_expected = true;

	free(answer);
	return status;
}

int idle(struct atsha_handle *handle) {
	int status;
	int tries = TRY_SEND_RECV_ON_COMM_ERROR;

	while (true) {
		switch (handle->bottom_layer) {

			case BOTTOM_LAYER_EMULATION:
				handle->wake_is_expected = false;
				return ATSHA_ERR_OK; //Idle is dummy in implementation. Always is successful.
				break;
			case BOTTOM_LAYER_NI2C:
				status = ni2c_idle(handle);
				break;
		}

		if (status == ATSHA_ERR_OK) {
			handle->wake_is_expected = false;
			return status;
		}
		if (tries < 0) return status;
	}
}

int command(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer) {
	int status;
	int tries = TRY_SEND_RECV_ON_COMM_ERROR + 1; //+1 will be eliminated after first iteration

	while (tries >= 0) {
		tries--;

		switch (handle->bottom_layer) {
			case BOTTOM_LAYER_EMULATION:
				return emul_command(handle, raw_packet, answer);
				break;
			case BOTTOM_LAYER_NI2C:
				status = ni2c_command(handle, raw_packet, answer);
				break;
		}

		if (status == ATSHA_ERR_OK) {
			//Check packet consistency and status code
			if (!check_packet(*answer)) {
				free(*answer);
				*answer = NULL;
				log_message("communication: command: CRC doesn't match.");
				status = ATSHA_ERR_COMMUNICATION;
				try_send_and_recv_sleep(handle);
				continue;
			}

			if ((*answer)[0] == 4) { //Messages with length 4 are always status codes
				unsigned char atsha204_status = (*answer)[1];
				bool go_trough = true;
				if (atsha204_status == ATSHA204_STATUS_PARSE_ERROR) {
					log_message("communication: command: Bad ATSHA204 status: Parse error.");
					go_trough = false;
				} else if (atsha204_status == ATSHA204_STATUS_EXEC_ERROR) {
					log_message("communication: command: Bad ATSHA204 status: Execution error.");
					go_trough = false;
				} else if (atsha204_status == ATSHA204_STATUS_COMMUNICATION_ERROR) {
					log_message("communication: command: Bad ATSHA204 status: Communication error.");
					go_trough = false;
				} else if (atsha204_status == ATSHA204_STATUS_WAKE_OK) {
					log_message("communication: command: Bad ATSHA204 status: Wake OK (undocumented; inadmissible in common communication).");
					tries = -1; //Kill immediately
					go_trough = false;
				} //The rest of status codes are distributed

				if (!go_trough) {
					free(*answer);
					*answer = NULL;
					status = ATSHA_ERR_BAD_COMMUNICATION_STATUS;
					try_send_and_recv_sleep(handle);
					continue;
				}
			}

			break;
		} else {
			try_send_and_recv_sleep(handle);
		}
	}

	return status;
}
