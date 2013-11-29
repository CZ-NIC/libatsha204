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

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <stdbool.h>

/**
 * \file communication.h
 * \brief Wrappers for different implementations of a bottom layer
 */

/**
 * \brief Wrapper for layer-dependent implementation of wake command
 */
int wake(struct atsha_handle *handle);
/**
 * \brief Wrapper for layer-dependent implementation of idle command
 */
int idle(struct atsha_handle *handle);
/**
 * \brief Wrapper for layer-dependent implementation of data exchange
 * \param raw_packet Command for the device
 * \param [out] answer Response from the device
 */
int command(struct atsha_handle *handle, unsigned char *raw_packet, unsigned char **answer);

#endif //COMMUNICATION_H
