// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2024 Dan Halbert for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include <stdint.h>

#include "py/obj.h"
#include "py/runtime.h"

#include "__init__.h"

//| """Cryptography operations
//|
//| """


// Called when cryptio is imported.
static mp_obj_t cryptio___init__(void) {
    common_hal_cryptio_init(true);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_0(cryptio___init___obj, cryptio___init__);

static const mp_rom_map_elem_t cryptio_module_globals_table[] = {
    {MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_cryptio)},

    // Initialization
    { MP_ROM_QSTR(MP_QSTR___init__),    MP_ROM_PTR(&wifi___init___obj) },
};

static MP_DEFINE_CONST_DICT(cryptio_module_globals, cryptio_module_globals_table);

const mp_obj_module_t cryptio_module = {
    .base = {&mp_type_module},
    .globals = (mp_obj_dict_t *)&cryptio_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_cryptio, cryptio_module);
