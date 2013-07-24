#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> //close()
#include<sys/file.h> //open()
#include<fcntl.h>
#include<errno.h>
#include<string.h> //strerror()
#include<stdint.h>
#include<stdbool.h>

#include "tools.h"
#include "configuration.h"
#include "atsha204.h"
#include "error.h"
#include "layer_usb.h"

int wake(int dev) {
	int status;
	int tries = TRY_SEND_RECV_ON_COMM_ERROR;

	while (true) {
#ifdef USE_LAYER_USB
		status = usb_wake(dev);
#endif
		if (status == ERR_OK) return status;
		if (tries < 0) return status;
	}
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
	int tries = TRY_SEND_RECV_ON_COMM_ERROR;

	while (true) {
#ifdef USE_LAYER_USB
		status = usb_command(dev, raw_packet, answer);
#endif
		if (status == ERR_OK) return status;
		if (tries < 0) return status;
	}
}
