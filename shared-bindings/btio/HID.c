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

//| class HID:
//|     """Bluetooth Classic HID (Human Interface Device) support.
//|
//|     This class manages the HID devices and the Bluetooth connection to the host.
//|     """
//|

//|     devices: Tuple[Device, ...]
//|     """Tuple of all available Bluetooth Classic HID device interfaces.
//|     The default set of devices is ``Device.KEYBOARD, Device.MOUSE, Device.CONSUMER_CONTROL``,
//|     """
STATIC mp_obj_t btio_hid_get_devices(mp_obj_t self) {
    return common_hal_btio_hid_get_devices(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(btio_hid_get_devices_obj, btio_hid_get_devices);

MP_PROPERTY_GETTER(btio_hid_get_devices_obj,
    (mp_obj_t)&btio_hid_get_devices);

//|     def start(
//|         devices: Sequence[Device] = (Device.KEYBOARD, Device.Mouse, Device.ConsumerControl)
//|     ) -> None:
//|         """Specify which USB HID devices that will be available,
//|         and start making them available to Bluetooth Classic hosts.
//|
//|         :param Sequence devices: `Device` objects.
//|           If `devices` defaults to a standard set of devices.
//|         """
//|         ...
STATIC mp_obj_t btio_hid_start(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_devices };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_devices, MP_ARG_OBJ, { .obj = default_hid_devices_tuple }  },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_obj_t devices = args[ARG_devices].u_obj;

    const mp_int_t len = mp_obj_get_int(mp_obj_len(devices));
    for (mp_int_t i = 0; i < len; i++) {
        mp_obj_t item = mp_obj_subscr(devices, MP_OBJ_NEW_SMALL_INT(i), MP_OBJ_SENTINEL);
        mp_arg_validate_type(item, &btio_hid_device_type, MP_QSTR___class__);
    }

    common_hal_btio_hid_start(devices);

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(btio_hid_start_obj, 1, btio_hid_start);

//|     def stop() -> None:
//|         """Stop Bluetooth HID communication."""
//|         ...
//|
STATIC mp_obj_t btio_hid_stop(void) {
    common_hal_btio_hid_stop();
}
MP_DEFINE_CONST_FUN_OBJ_0(btio_hid_stop_obj, btio_hid_stop);

STATIC const mp_rom_map_elem_t btio_hid_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_start),         MP_ROM_PTR(&btio_hid_start_obj) },
    { MP_ROM_QSTR(MP_QSTR_stop),          MP_ROM_PTR(&btio_hid_stop_obj) },

    // Properties
    { MP_ROM_QSTR(MP_QSTR_devices),       MP_ROM_PTR( },
};

STATIC MP_DEFINE_CONST_DICT(btio_hid_locals_dict, btio_hid_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    btio_hid_type,
    MP_QSTR_HID,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    locals_dict, &btio_hid_locals_dict
    );
