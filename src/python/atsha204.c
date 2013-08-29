#include <Python.h>
#include <dlfcn.h>
#include "../libatsha204/atsha204.h"

/*
 * As linking to other libraries from python extensions is tricky,
 * we do fully dynamic loading of the library with dlopen.
 */

static PyObject *atsha_emulate_hmac(PyObject *self, PyObject *args) {
	(void) self;
	int size_serial, size_key, size_challenge;
	unsigned char slot_id;
	const uint8_t *serial, *key, *challenge;
	if (!PyArg_ParseTuple(args, "bs#s#s#", &slot_id, &serial, &size_serial, &key, &size_key, &challenge, &size_challenge)) {
		return NULL;
	}

	if (32 != size_key) {
		PyErr_SetString(PyExc_ValueError, "key length should be 32!");
		return NULL;
	}
	if (32 != size_challenge) {
		PyErr_SetString(PyExc_ValueError, "challenge length should be 32!");
		return NULL;
	}

	struct atsha_handle *crypto = atsha_open_server_emulation(slot_id, serial, key);
	if (NULL == crypto) {
        PyErr_SetString(PyExc_RuntimeError, "failed to initialize crypto library");
        return NULL;
    }
	atsha_big_int challenge_s, response_s;
	challenge_s.bytes = 32;
	memcpy(challenge_s.data, challenge, 32);
	int result = atsha_challenge_response(crypto, challenge_s, &response_s);
	if (ATSHA_ERR_OK != result) {
		PyErr_SetString(PyExc_ValueError, atsha_error_name(result));
		return NULL;
	}
	atsha_close(crypto);
	return Py_BuildValue("s#", response_s.data, (int) response_s.bytes);
}

static PyObject *atsha_do_hmac(PyObject *self, PyObject *args) {
	(void) self;
	int size_challenge;
	const uint8_t *challenge;
	if (!PyArg_ParseTuple(args, "s#", &challenge, &size_challenge)) {
		return NULL;
	}

	if (32 != size_challenge) {
		PyErr_SetString(PyExc_ValueError, "challenge length should be 32!");
		return NULL;
	}

	struct atsha_handle *crypto = atsha_open();
	if (NULL == crypto) {
        PyErr_SetString(PyExc_RuntimeError, "failed to initialize crypto library");
        return NULL;
    }
	atsha_big_int challenge_s, response_s;
	challenge_s.bytes = 32;
	memcpy(challenge_s.data, challenge, 32);
	int result = atsha_challenge_response(crypto, challenge_s, &response_s);
	if (ATSHA_ERR_OK != result) {
		PyErr_SetString(PyExc_ValueError, atsha_error_name(result));
		return NULL;
	}
	atsha_close(crypto);
	return Py_BuildValue("s#", response_s.data, (int) response_s.bytes);
}

static PyMethodDef atsha_methods[] = {
	{
		"emulate_hmac",
		atsha_emulate_hmac,
		METH_VARARGS,
		"Emulates computation of hmac from atsh204 cryptographic chip.\n"
		"hmac(slot_id, serial, key, challenge) where len(key)=32 and len(challenge)=32"
	},
	{
		"hmac",
		atsha_do_hmac,
		METH_VARARGS,
		"Computes a hmac using atsh204 cryptographic chip.\n"
		"hmac(challenge) where len(challenge)=32\n"
		"\n"
		"Note than internet connection is required to obtain a slot_id from DNS"
	},
	{NULL}
};

PyMODINIT_FUNC initatsha204(void) {
	Py_InitModule("atsha204", atsha_methods);
}
