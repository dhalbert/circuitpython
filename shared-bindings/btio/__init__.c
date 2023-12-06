/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2023 Dan Halbert for Adafruit Industries
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "shared-bindings/btio/__init__.h"
#include "shared-bindings/btio/HID.h"

//| """
//| The `btio` module provides Bluetooth Classic (BR/EDR) communication
//| using predefined Bluetooth profiles.
//| """
//|
//| hid: HID
//| """Bluetooth Classic HID Profile support.
//| This object is the sole instance of `btio.HID`."""

// Called when btio is imported.
STATIC mp_obj_t btio___init__(void) {
    common_hal_btio_init();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(btio___init___obj, btio___init__);

STATIC const mp_rom_map_elem_t btio_module_globals_table[] = {
    // Name
    { MP_ROM_QSTR(MP_QSTR___name__),    MP_ROM_QSTR(MP_QSTR_btio) },

    // Initialization
    { MP_ROM_QSTR(MP_QSTR___init__),    MP_ROM_PTR(&btio___init___obj) },

    // Classes
    { MP_ROM_QSTR(MP_QSTR_HID),       MP_ROM_PTR(&btio_hid_type) },

    // Properties
    { MP_ROM_QSTR(MP_QSTR_hid),       MP_ROM_PTR(&common_hal_btio_hid_obj) },
};
STATIC MP_DEFINE_CONST_DICT(btio_module_globals, btio_module_globals_table);

const mp_obj_module_t btio_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&btio_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_btio, btio_module);
