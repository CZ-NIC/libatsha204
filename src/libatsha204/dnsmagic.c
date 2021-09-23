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

#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

//DNS Resolving
//#include <unbound.h>

#include "configuration.h"
#include "atsha204consts.h"
#include "atsha204.h"
#include "tools.h"
#include "api.h"

/*
 * viz IANA
 * This is not documented in unbound
 * Unbound refers to IANA too
 */
#define CLASS_INET 1
#define TYPE_TXT 16

/*
 * Global variable with configuration and some initial config values.
 */
atsha_configuration g_config;

/*
 * Get decimal number from its string representation
 */
static uint32_t number_from_string(size_t len, char *str) {
	uint32_t res = 0;
	unsigned char digit = 0;

	for (size_t i = 0; i < len; i++) {
		digit = str[i] - '0';
		res *= 10;
		res += digit;
	}

	return res;
}

/*
 * Use linunbound for DNS resolving of TXT record
 */
//static bool resolve_key(uint32_t *offset) {
//	struct ub_result* result;
//    struct ub_ctx *ctx = ub_ctx_create();
//	int retval;
//	char strbuff[BUFFSIZE_DNSMAGIC_ERRSTRLEN];
//
//    if (!ctx) {
//		log_message("dnsmagic: libunbound: create context error");
//		return false;
//	}
//
//	/*
//	 * Set resolvconf to NULL and libunbound will respect settings of OS
//	 * Eventually: ub_ctx_set_fwd(ctx, "8.8.8.8") set address of requested resolver/forwarder
//	 */
//	retval = ub_ctx_resolvconf(ctx, NULL);
//	if (retval != 0)  {
//		log_message("dnsmagic: libunbound: reset configuration error");
//		snprintf(strbuff, BUFFSIZE_DNSMAGIC_ERRSTRLEN, "libunbound returned %d status code with explanation: %s and errno: %s\n", retval, ub_strerror(retval), strerror(errno));
//		log_message(strbuff);
//
//		return false;
//	}
//
//	retval = ub_resolve(ctx, DEFAULT_DNS_RECORD_FIND_KEY, TYPE_TXT, CLASS_INET, &result);
//	if (retval != 0) {
//		log_message("dnsmagic: libunbound: resolve error");
//		snprintf(strbuff, BUFFSIZE_DNSMAGIC_ERRSTRLEN, "libunbound returned %d status code with explanation: %s\n", retval, ub_strerror(retval));
//		log_message(strbuff);
//		ub_ctx_delete(ctx);
//		return false;
//	}
//
//	if (result->havedata) {
//		*offset = (unsigned char) number_from_string(result->data[0][0], (result->data[0] + 1));
//		ub_resolve_free(result);
//		ub_ctx_delete(ctx);
//		return true;
//	}
//
//	log_message("dnsmagic: libunbound: no data in answer");
//
//	ub_resolve_free(result);
//	ub_ctx_delete(ctx);
//
//	return false;
//}

unsigned char atsha_find_slot_number(struct atsha_handle *handle) {
	/*
	if (handle->is_srv_emulation == true) {
		return handle->slot_id;
	}

	uint32_t offset;
	if (!resolve_key(&offset)) {
		return DNS_ERR_CONST;
	}

	if (!handle->key_origin_cached) {
		atsha_big_int number;
		if (atsha_raw_otp_read(handle, ATSHA204_OTP_MEMORY_MAP_ORIGIN_KEY_SET, &number) != ATSHA_ERR_OK) {
			log_message("dnsmagic: find_slot_number: read key origin from OTP memory");
			return DNS_ERR_CONST;
		}

		handle->key_origin = uint32_from_4_bytes(number.data);
		handle->key_origin_cached = true;
	}


	return (unsigned char)(offset - handle->key_origin);
	*/
	return 1;
}
