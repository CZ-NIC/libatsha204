/*
 * libatsha204 is small library and set of tools for Amel ATSHA204 crypto chip
 *
 * Copyright (C) 2013 CZ.NIC, z.s.p.o. (http://www.nic.cz/)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LAYER_I2C_H
#define LAYER_I2C_H

#include <stdbool.h>

void i2c_wait();
int i2c_wake(struct atsha_handle *handle, unsigned char **answer);
int i2c_idle(struct atsha_handle *handle);
int i2c_command(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer);

#endif //LAYER_I2C_H
