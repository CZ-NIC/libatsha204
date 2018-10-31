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

#include <Python.h>
#include <dlfcn.h>
#include "../libatsha204/atsha204.h"
#include <stdio.h>

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
		atsha_close(crypto);
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
		atsha_close(crypto);
		return NULL;
	}
	atsha_close(crypto);
	return Py_BuildValue("s#", response_s.data, (int) response_s.bytes);
}

static PyObject *get_serial(PyObject *self, PyObject *args) {
	(void) self;

	if (!PyArg_ParseTuple(args, "")) {
		return NULL;
	}

	struct atsha_handle *crypto = atsha_open();
	if (NULL == crypto) {
		PyErr_SetString(PyExc_RuntimeError, "failed to initialize crypto library");
		return NULL;
	}

	atsha_big_int abi_serial;
	int result = atsha_serial_number(crypto, &abi_serial);
	if (ATSHA_ERR_OK != result) {
		const char *err_msg = atsha_error_name(result);
		char *first_msg = "failed to get serial number";
		char msg[strlen(first_msg) + 3 + strlen(err_msg) + 1];
		sprintf(msg, "%s - %s", first_msg, err_msg);
		PyErr_SetString(PyExc_RuntimeError, msg);
		atsha_close(crypto);
		return NULL;
	}
	atsha_close(crypto);
	return Py_BuildValue("s#", abi_serial.data, (int) abi_serial.bytes);
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
	{
		"get_serial",
		get_serial,
		METH_VARARGS,
		"Get serial number of the atsh204 cryptographic chip.\n"
	},
	{NULL}
};

struct module_state {
	PyObject *error;
};

#define GETSTATE(m) ((struct module_state*)PyModule_GetState(m))

static int atsha_traverse(PyObject *m, visitproc visit, void *arg) {
	Py_VISIT(GETSTATE(m)->error);
	return 0;
}

static int atsha_clear(PyObject *m) {
	Py_CLEAR(GETSTATE(m)->error);
	return 0;
}

static struct PyModuleDef moduledef = {
	PyModuleDef_HEAD_INIT,
	"atsha204",
	NULL,
	sizeof(struct module_state),
	atsha_methods,
	NULL,
	atsha_traverse,
	atsha_clear,
	NULL
};

PyMODINIT_FUNC
PyInit_atsha204(void)
{
	PyObject *module = PyModule_Create(&moduledef);
	if (module == NULL) {
		return NULL;
	}
	struct module_state *st = GETSTATE(module);

	st->error = PyErr_NewException("atsha204.Error", NULL, NULL);
	if (st->error == NULL) {
		Py_DECREF(module);
		return NULL;
	}

	return module;
}
