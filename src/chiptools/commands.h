#ifndef COMMANDS_H
#define COMMANDS_H

#include "../libatsha204/atsha204.h"

void dump_config(struct atsha_handle *handle);
void dump_data(struct atsha_handle *handle);
void dump_otp(struct atsha_handle *handle);

#endif // COMMANDS_H
