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
#include "tools.h"
#include "operations.h"

static int device_fd;
static void (*warn_callback)(const char* msg) = NULL;

void log_warning(const char* msg) {
	if (warn_callback != NULL) {
		warn_callback(msg);
	}
}

void set_warn_callback(void (*clb)(const char* msg)) {
	warn_callback = clb;
}

int dev_rev(uint32_t *revision) {
	(void) revision;
	int status;
	unsigned char *packet;
	unsigned char *answer = NULL;

	//Wakeup device
	status = wake(device_fd);
	if (status != ERR_OK) return status;

	packet = op_dev_rev();
	if (!packet) return ERR_MEMORY_ALLOCATION_ERROR;

	status = command(device_fd, packet, &answer);
	if (status != ERR_OK) {
		free(packet);
		free(answer);
		return status;
	}

	*revision = op_dev_rev_recv(answer);

	//Let device sleep
	status = idle(device_fd);
	if (status != ERR_OK) {
		log_warning(WARN_MSG_IDLE_NOT_CONFIRMED);
	}

	free(packet);
	free(answer);

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

	device_fd = open(argv[1], O_RDWR);
	if (device_fd == -1) {
		fprintf(stderr, "Couldn't open %s devidce.\n", argv[1]);
		return 1;
	}

	set_warn_callback(testing_warn_callback);

	int status;

	// Get Revision
	uint32_t rev;
	status = dev_rev(&rev);
	fprintf(stderr, "Status: %s\n", error_name(status));
	fprintf(stderr, "Revision: %u\n", rev);

	close(device_fd);

	return 0;
}
