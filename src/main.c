#include<stdio.h>
#include<stdlib.h>
#include<unistd.h> //close()
#include<sys/file.h> //open()
#include<fcntl.h>
#include<errno.h>
#include<string.h> //strerror()
#include<stdint.h>

#include "main.h"
#include "layer_usb.h"
#include "error.h"

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s hidraw device path\n", argv[0]);
		return 1;
	}

	int fd = open(argv[1], O_RDWR);
	if (fd == -1) {
		fprintf(stderr, "Couldn't open %s devidce.\n", argv[1]);
		return 1;
	}

	//device_comm_talk(fd);
	int status = usb_wake(fd);
	fprintf(stderr, "Status %s\n", error_name(status));

	close(fd);

	return 0;
}
