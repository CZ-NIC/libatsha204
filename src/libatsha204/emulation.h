#ifndef EMULATION_H
#define EMULATION_H

#include<stdbool.h>

int emul_command(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer);

#endif //EMULATION_H
