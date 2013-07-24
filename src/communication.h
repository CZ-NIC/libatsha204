#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <stdbool.h>

bool check_packet(unsigned char *packet);
void op_dev_rev();

#endif //COMMUNICATION_H
