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

uint64_t dev_rev() {
	return 0;
}

bool wake_emul() {
	return true;
}
