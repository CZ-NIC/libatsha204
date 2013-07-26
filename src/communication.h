#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <stdbool.h>

int wake(int dev);
int idle(int dev);
int command(int dev, unsigned char *raw_packet, unsigned char **answer);

#endif //COMMUNICATION_H
