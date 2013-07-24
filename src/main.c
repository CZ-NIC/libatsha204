#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> //close()
#include<sys/file.h> //open()
#include<fcntl.h>
#include<errno.h>
#include<string.h> //strerror()
#include<stdint.h>

#include "main.h"
#include "error.h"
#include "communication.h"

static int device_fd;
static void (*warn_callback)(const char* msg) = NULL;

void log_warning(const char* msg) {
	if (warn_callback != NULL) {
		warn_callback(msg);
	}
}

void set_war_callback(void (*clb)(const char* msg)) {
	warn_callback = clb;
}

int dev_rev(uint32_t *revision) {
	int status;
	unsigned char *packet;
	unsigned char **answer;

	//Wakeup device
	status = wake(device_fd);
	if (status != ERR_OK) return status;
/*
	packet = op_dev_rev();
	if (!packet) return ERR_MEMORY_ALLOCATION_ERROR;

	status = usb_command(device_fd, packet, answer);
	if (status != ERR_OK) {
		free(send);
		return status;
	}
*/
	//Let device sleep
	status = idle(device_fd);
	if (status != ERR_OK) return status;

	return ERR_OK;
}

/*
 * From this point bellow is code just for testing and it simulation of
 * some application implementing this library.
 */

void testing_warn_callback(const char *msg) {
	fprintf(stderr, "Warning: %s\n", msg);
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s hidraw device path\n", argv[0]);
		return 1;
	}

	int device_fd = open(argv[1], O_RDWR);
	if (device_fd == -1) {
		fprintf(stderr, "Couldn't open %s devidce.\n", argv[1]);
		return 1;
	}



	int status;


	close(device_fd);

	return 0;
}
