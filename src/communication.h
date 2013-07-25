#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <stdbool.h>

int wake(int dev);
int idle(int dev);
int command(int dev, unsigned char *raw_packet, unsigned char **answer, bool check_status_code);

#endif //COMMUNICATION_H
