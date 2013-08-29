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
