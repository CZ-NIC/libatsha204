#ifndef MAIN_H
#define MAIN_H

#include<stdint.h>

typedef struct {
	size_t bytes;
	unsigned char *data;
} big_int;

int dev_rev(uint32_t *revision);

#endif //MAIN_H
