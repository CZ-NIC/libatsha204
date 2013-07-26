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
#include "main.h"

extern atsha_configuration g_config;

int wake(int dev) {
	int status;
	int tries = TRY_SEND_RECV_ON_COMM_ERROR + 1; //+1 will be eliminated after first iteration
	unsigned char *answer = NULL;

	while (tries >= 0) {
		tries--;
////////////////////////////////////////////////////////////////////////
#ifdef USE_LAYER_USB
		status = usb_wake(dev, &answer);
#endif
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
		}
	}

	free(answer);
	return status;
}

int idle(int dev) {
	int status;
	int tries = TRY_SEND_RECV_ON_COMM_ERROR;

	while (true) {
#ifdef USE_LAYER_USB
		status = usb_idle(dev);
#endif
		if (status == ATSHA_ERR_OK) return status;
		if (tries < 0) return status;
	}
}

int command(int dev, unsigned char *raw_packet, unsigned char **answer, bool check_status_code) {
	int status;
	int tries = TRY_SEND_RECV_ON_COMM_ERROR + 1; //+1 will be eliminated after first iteration

	while (tries >= 0) {
		tries--;
////////////////////////////////////////////////////////////////////////
#ifdef USE_LAYER_USB
		status = usb_command(dev, raw_packet, answer);
#endif
////////////////////////////////////////////////////////////////////////
		if (status == ATSHA_ERR_OK) {
			//Check packet consistency and status code
			if (!check_packet(*answer)) {
				free(*answer);
				*answer = NULL;
				if (g_config.verbose) log_message("ERR: Command: CRC doesn't match.");
				status = ATSHA_ERR_COMMUNICATION;
				continue;
			}

			if (check_status_code) {
				if ((*answer)[1] != ATSHA204_STATUS_SUCCES) {
					if (g_config.verbose) log_message("ERR: Command: Check ATSHA204 status code was requested and status code is different from SUCCESS (0x00)");
					//Parse ATSHA_ERRor is really bad and it isn't user's or device fail
					assert(!((*answer)[1] == ATSHA204_STATUS_PARSE_ATSHA_ERROR));
					free(*answer);
					*answer = NULL;
					status = ATSHA_ERR_COMMUNICATION;
					continue;
				}
			}

			break;
		}
	}

	return status;
}
