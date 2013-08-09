#include<stdint.h>
#include<stdbool.h>

//DNS Resolving
#include <unbound.h>

#include "configuration.h"
#include "atsha204consts.h"
#include "atsha204.h"
#include "api.h"

/**
 * viz IANA
 * This is not documented in unbound
 * Unbound refers to IANA too
 */
#define CLASS_INET 1
#define TYPE_TXT 16

/**
 * Global variable with configuration and some initial config values.
 */
atsha_configuration g_config;

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

static bool resolve_key(uint32_t *offset) {
	struct ub_result* result;
    struct ub_ctx *ctx = ub_ctx_create();

    if (!ctx) {
		log_message("dnsmagic: libunbound: create context error");
		return false;
	}

	if (ub_resolve(ctx, DEFAULT_DNS_RECORD_FIND_KEY, TYPE_TXT, CLASS_INET, &result) != 0) {
		log_message("dnsmagic: libunbound: resolve error");
		ub_ctx_delete(ctx);
		return false;
	}

	if (result->havedata) {
		*offset = (unsigned char) number_from_string(result->data[0][0], (result->data[0] + 1));
		ub_resolve_free(result);
		ub_ctx_delete(ctx);
		return true;
	}

	ub_resolve_free(result);
	ub_ctx_delete(ctx);
	log_message("dnsmagic: libunbound: no data in answer");
	return false;
}

unsigned char atsha_find_slot_number(struct atsha_handle *handle) {
	if (handle->is_srv_emulation == true) {
		/**
		 * It is not important what is the returned value.
		 * This line is just saving server resources
		 */
		return 0;
	}

	uint32_t offset;
	if (!resolve_key(&offset)) {
		return DNS_ERR_CONST;
	}

	return (unsigned char)(offset - handle->key_origin);
}
