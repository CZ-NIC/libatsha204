#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> //close()
#include<sys/file.h> //open()
#include<fcntl.h>
#include<errno.h>
#include<string.h> //strerror()
#include<stdint.h>
#include<stdbool.h>
#include<assert.h>

#include "tools.h"
#include "configuration.h"
#include "atsha204.h"
#include "error.h"
#include "layer_usb.h"

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
		if (status == ERR_OK) {
			//Check packet consistency and check wake confirmation
			if (!check_packet(answer)) {
				status = ERR_COMMUNICATION;
				continue;
			}

			if (answer[1] != ATSHA204_STATUS_WAKE_OK) {
				status = ERR_WAKE_NOT_CONFIRMED;
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
		if (status == ERR_OK) return status;
		if (tries < 0) return status;
	}
}

int command(int dev, unsigned char *raw_packet, unsigned char **answer) {
	int status;
	int tries = TRY_SEND_RECV_ON_COMM_ERROR + 1; //+1 will be eliminated after first iteration

	while (tries >= 0) {
		tries--;
////////////////////////////////////////////////////////////////////////
#ifdef USE_LAYER_USB
		status = usb_command(dev, raw_packet, answer);
#endif
////////////////////////////////////////////////////////////////////////
		if (status == ERR_OK) {
			//Check packet consistency and status code
			if (!check_packet(*answer)) {
				status = ERR_COMMUNICATION;
				continue;
			}

			if ((*answer)[1] != ATSHA204_STATUS_SUCCES) {
				//Parse error is really bad and it isn't user's or device fail
				assert(!((*answer)[1] == ATSHA204_STATUS_PARSE_ERROR));
				status = ERR_COMMUNICATION;
				continue;
			}

			break;
		}
	}

	return status;
}
