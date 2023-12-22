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

#include "py/obj.h"
#include "py/mphal.h"
#include "py/runtime.h"

#include "shared-bindings/bt_hid/__init__.h"
#include "shared-bindings/bt_hid/Device.h"

//| """
//| The `bt_hid` module supports Bluetooth Classic (BR/EDR)
//| Human Interface Device (HID) Profile
//| """
//|

// Called when bt_hid is imported.
STATIC mp_obj_t bt_hid___init__(void) {
    common_hal_bt_hid_init();
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(bt_hid___init___obj, bt_hid___init__);

//| devices: Tuple[Device, ...]
//| """Tuple of all available Bluetooth Classic HID device interfaces.
//| The default set of devices is ``Device.KEYBOARD, Device.MOUSE, Device.CONSUMER_CONTROL``,
//|
//| If a boot device is enabled by `bt_hid.start()`, *and* the host has requested a boot device,
//| the `devices` tuple is **replaced** with a single-element tuple
//| containing a `Device` that describes the boot device chosen (keyboard or mouse).
//| The request for a boot device overrides any other HID devices.
//| """
//|

//| def start(
//|     devices: Sequence[Device] = (Device.KEYBOARD, Device.Mouse, Device.ConsumerControl),
//|     boot_device: int = 0,
//| ) -> None:
//|     """Specify which HID devices that will be available,
//|     and start making them available to Bluetooth Classic hosts.
//|
//|     :param Sequence devices: `Device` objects.
//|       If `devices` is not given, it defaults to a standard set of devices.
//|     :param int boot_device: If non-zero, inform the host that support for a
//|       a boot HID device is available.
//|       If ``boot_device=1``, a boot keyboard is available.
//|       If ``boot_device=2``, a boot mouse is available. No other values are allowed.
//|       See below.
//|
//|     If you enable too many devices at once, you will run out of USB endpoints.
//|     The number of available endpoints varies by microcontroller.
//|     CircuitPython will go into safe mode after running ``boot.py`` to inform you if
//|     not enough endpoints are available.
//|
//|     **Boot Devices**
//|
//|     Boot devices implement a fixed, predefined report descriptor, defined in
//|     https://www.usb.org/sites/default/files/hid1_12.pdf, Appendix B. A host
//|     can request to use the boot device if the Bluetooth HID device says it is available.
//|     Usually only a BIOS or other kind of limited-functionality
//|     host needs boot keyboard support.
//|     Many Bluetooth Classic hosts do not support boot devices.
//|
//|     For example, to make a boot keyboard available, you can use this code::
//|
//|       bt_hid.enable((Device.KEYBOARD), boot_device=1)  # 1 for a keyboard
//|
//|     If the host requests the boot keyboard, the report descriptor provided by `Device.KEYBOARD`
//|     will be ignored, and the predefined report descriptor will be used.
//|     But if the host does not request the boot keyboard,
//|     the descriptor provided by `Device.KEYBOARD` will be used.
//|
//|     The HID boot device must usually be the first or only device presented by CircuitPython.
//|     The HID device will be USB interface number 0.
//|     To make sure it is the first device, disable other USB devices, including CDC and MSC (CIRCUITPY).
//|     If you specify a non-zero ``boot_device``, and it is not the first device, CircuitPython
//|     will raise an exception.
//|     """
//|     ...
STATIC mp_obj_t bt_hid_start(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_devices, ARG_boot_device };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_devices, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_boot_device, MP_ARG_INT, {.u_int = 0} },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_obj_t devices = args[ARG_devices].u_obj;

    if (devices != mp_const_none) {
        const mp_int_t len = mp_obj_get_int(mp_obj_len(devices));
        for (mp_int_t i = 0; i < len; i++) {
            mp_obj_t item = mp_obj_subscr(devices, MP_OBJ_NEW_SMALL_INT(i), MP_OBJ_SENTINEL);
            mp_arg_validate_type(item, &bt_hid_device_type, MP_QSTR___class__);
        }
    }

    uint8_t boot_device =
        (uint8_t)mp_arg_validate_int_range(args[ARG_boot_device].u_int, 0, 2, MP_QSTR_boot_device);

    // If devices is None, use the default device tuple.
    if (!common_hal_bt_hid_start(devices, boot_device)) {
        mp_raise_RuntimeError_varg(MP_ERROR_TEXT("%q failed"), MP_QSTR_start);
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_KW(bt_hid_start_obj, 0, bt_hid_start);

//|     def stop() -> None:
//|         """Stop Bluetooth HID communication."""
//|         ...
//|
STATIC mp_obj_t bt_hid_stop(void) {
    if (!common_hal_bt_hid_stop()) {
        mp_raise_RuntimeError_varg(MP_ERROR_TEXT("%q failed"), MP_QSTR_stop);
    }

    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_0(bt_hid_stop_obj, bt_hid_stop);

//| def get_boot_device() -> int:
//|     """
//|     :return: the boot device requested by the host, if any.
//|       Returns 0 if the host did not request a boot device, or if `bt_hid.start()`
//|       was called with ``boot_device=0``, the default, which disables boot device support.
//|       If the host did request a boot device,
//|       returns the value of ``boot_device`` set in `bt_hid.enable()`:
//|       ``1`` for a boot keyboard, or ``2`` for boot mouse.
//|       However, the standard devices provided by CircuitPython, `Device.KEYBOARD` and `Device.MOUSE`,
//|       describe reports that match the boot device reports, so you don't need to check this
//|       if you are using those devices.
//|     :rtype int:
//|     """
//|
STATIC mp_obj_t bt_hid_get_boot_device(void) {
    return MP_OBJ_NEW_SMALL_INT(common_hal_bt_hid_get_boot_device());
}
MP_DEFINE_CONST_FUN_OBJ_0(bt_hid_get_boot_device_obj, bt_hid_get_boot_device);

STATIC mp_map_elem_t bt_hid_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),        MP_OBJ_NEW_QSTR(MP_QSTR_bt_hid) },
    { MP_ROM_QSTR(MP_QSTR___init__),        MP_OBJ_FROM_PTR(&bt_hid___init___obj) },
    { MP_ROM_QSTR(MP_QSTR_Device),          MP_OBJ_FROM_PTR(&bt_hid_device_type) },
    { MP_ROM_QSTR(MP_QSTR_devices),         mp_const_none },
    { MP_ROM_QSTR(MP_QSTR_start),           MP_OBJ_FROM_PTR(&bt_hid_start_obj) },
    { MP_ROM_QSTR(MP_QSTR_stop),            MP_OBJ_FROM_PTR(&bt_hid_stop_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_boot_device), MP_OBJ_FROM_PTR(&bt_hid_get_boot_device_obj) },
};
STATIC MP_DEFINE_MUTABLE_DICT(bt_hid_module_globals, bt_hid_module_globals_table);

const mp_obj_module_t bt_hid_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&bt_hid_module_globals,
};

void bt_hid_set_devices(mp_obj_t devices) {
    mp_map_elem_t *elem =
        mp_map_lookup(&bt_hid_module_globals.map, MP_ROM_QSTR(MP_QSTR_devices), MP_MAP_LOOKUP);
    if (elem) {
        elem->value = devices;
    }
}

MP_REGISTER_MODULE(MP_QSTR_bt_hid, bt_hid_module);
