#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <stdbool.h>

int wake(struct atsha_handle *handle);
int idle(struct atsha_handle *handle);
int command(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer);

#endif //COMMUNICATION_H
