/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Scott Shawcroft for Adafruit Industries
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

#include "py/objproperty.h"
#include "shared-bindings/usb_hid/Device.h"
#include "py/runtime.h"

//| class Device:
//|     """HID Device specification"""
//|
//|     def __init__(self, *, descriptor: ReadableBuffer, usage_page: int, usage: int, in_report_length: int, out_report_length: int = 0, report_id_index: Optional[int], boot: bool=False) -> None:
//|         """Create a description of a USB HID device. The actual device is created when you
//|         pass a `Device` to `usb_hid.enable()`.
//|
//|         :param ReadableBuffer report_descriptor: The USB HID Report descriptor bytes. The descriptor is not
//|           not verified for correctness; it is up to you to make sure it is not malformed.
//|         :param int usage_page: The Usage Page value from the descriptor. Must match what is in the descriptor.
//|         :param int usage: The Usage value from the descriptor. Must match what is in the descriptor.
//|         :param int in_report_length: Size in bytes of the HID report sent to the host.
//|           "In" is with respect to the host.
//|         :param int out_report_length: Size in bytes of the HID report received from the host.
//|           "Out" is with respect to the host. If no reports are expected, use 0.
//|         :param int report_id_index: position of byte in descriptor that contains the Report ID.
//|           A Report ID will be assigned when the device is created. If there is no
//|           Report ID, use ``None``. The report ID will be omitted if this device is the only
//|           device in its parent HID descriptor.
//|         :param bool boot: Present this device as a boot protocol device if it is a mouse or keyboard
//|           and if it is the only device in its parent HID descriptor. If used as a boot device,
//|           the report descriptor presented is ignored, and a standard one is used instead.
//|         """
//|         ...
//|
//|     KEYBOARD: Device
//|     """Typical keyboard device supporting keycodes 0x00-0xDD, modifiers 0xE-0xE7, and five LED indicators.
//|     Can be used as a boot device. The `Device` values are shown below.
//|
//|     .. code-block::
//|
//|       KEYBOARD = Device(descriptor=<see below>, usage_page=0x01, usage=0x06,
//|                         in_report_length=8, out_report_length=1, report_id_index=7, boot=True)
//|
//|       Keyboard report descriptor:
//|
//|       0x05, 0x01,  # Usage Page (Generic Desktop Ctrls)
//|       0x09, 0x06,  # Usage (Keyboard)
//|       0xA1, 0x01,  # Collection (Application)
//|       0x85, 0x--,  #   Report ID  [set at runtime, will be omitted if sole device]
//|       0x05, 0x07,  #   Usage Page (Kbrd/Keypad)
//|       0x19, 0xE0,  #   Usage Minimum (0xE0 = 224)
//|       0x29, 0xE7,  #   Usage Maximum (0xE7 = 231)
//|       0x15, 0x00,  #   Logical Minimum (0)
//|       0x25, 0x01,  #   Logical Maximum (1)
//|       0x75, 0x01,  #   Report Size (1)
//|       0x95, 0x08,  #   Report Count (8)
//|       0x81, 0x02,  #   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
//|       0x81, 0x01,  #   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
//|       0x19, 0x00,  #   Usage Minimum (0x00)
//|       0x29, 0xDD,  #   Usage Maximum (0xDD = 221)
//|       0x15, 0x00,  #   Logical Minimum (0)
//|       0x25, 0xDD,  #   Logical Maximum (0xDD = 221)
//|       0x75, 0x08,  #   Report Size (8)
//|       0x95, 0x06,  #   Report Count (6)
//|       0x81, 0x00,  #   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
//|       0x05, 0x08,  #   Usage Page (LEDs)
//|       0x19, 0x01,  #   Usage Minimum (Num Lock)
//|       0x29, 0x05,  #   Usage Maximum (Kana)
//|       0x15, 0x00,  #   Logical Minimum (0)
//|       0x25, 0x01,  #   Logical Maximum (1)
//|       0x75, 0x01,  #   Report Size (1)
//|       0x95, 0x05,  #   Report Count (5)
//|       0x91, 0x02,  #   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
//|       0x95, 0x03,  #   Report Count (3)
//|       0x91, 0x01,  #   Output (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
//|       0xC0,        # End Collection
//|
//|     The boot keyboard report descriptor is pre-defined in the USB standard and is very similar to the
//|     descriptor above. The pre-defined descriptor is used *instead* of the descriptor above.
//|     See page 59 in the `Device Class Definition for Human Interface Devices (HID) <https://www.usb.org/sites/default/files/hid1_11.pdf>`_.
//|     """
//|
//|     MOUSE: Device
//|     """Typical mouse device supporting five mouse buttons, X and Y relative movements from -127 to 127
//|     in each report, and a relative mouse wheel change from -127 to 127 in each report.
//|     Can be used as a boot device. The `Device` values are shown below.
//|
//|     .. code-block::
//|
//|       MOUSE = Device(descriptor=<see below>, usage_page=0x01, usage=0x02,
//|                      in_report_length=4, out_report_length=0, report_id_index=11, boot=True)
//|
//|       Mouse report descriptor:
//|
//|       0x05, 0x01,  # Usage Page (Generic Desktop Ctrls)
//|       0x09, 0x02,  # Usage (Mouse)
//|       0xA1, 0x01,  # Collection (Application)
//|       0x09, 0x01,  # Usage (Pointer)
//|       0xA1, 0x00,  #  Collection (Physical)
//|       0x85, 0x--,  #  Report ID  [set at runtime, will be omitted if sole device]
//|       0x05, 0x09,  #     Usage Page (Button)
//|       0x19, 0x01,  #     Usage Minimum (0x01)
//|       0x29, 0x05,  #     Usage Maximum (0x05)
//|       0x15, 0x00,  #     Logical Minimum (0)
//|       0x25, 0x01,  #     Logical Maximum (1)
//|       0x95, 0x05,  #     Report Count (5)
//|       0x75, 0x01,  #     Report Size (1)
//|       0x81, 0x02,  #     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
//|       0x95, 0x01,  #     Report Count (1)
//|       0x75, 0x03,  #     Report Size (3)
//|       0x81, 0x01,  #     Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
//|       0x05, 0x01,  #     Usage Page (Generic Desktop Ctrls)
//|       0x09, 0x30,  #     Usage (X)
//|       0x09, 0x31,  #     Usage (Y)
//|       0x15, 0x81,  #     Logical Minimum (-127)
//|       0x25, 0x7F,  #     Logical Maximum (127)
//|       0x75, 0x08,  #     Report Size (8)
//|       0x95, 0x02,  #     Report Count (2)
//|       0x81, 0x06,  #     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
//|       0x09, 0x38,  #     Usage (Wheel)
//|       0x15, 0x81,  #     Logical Minimum (-127)
//|       0x25, 0x7F,  #     Logical Maximum (127)
//|       0x75, 0x08,  #     Report Size (8)
//|       0x95, 0x01,  #     Report Count (1)
//|       0x81, 0x06,  #     Input (Data,Var,Rel,No Wrap,Linear,Preferred State,No Null Position)
//|       0xC0,        #   End Collection
//|       0xC0,        # End Collection
//|
//|     The boot mouse report descriptor is pre-defined in the USB standard and is similar to the
//|     descriptor above, except it only has three mouse buttons and no wheel. The X and Y
//|     ranges are also -127 to 127.
//|     The pre-defined descriptor is used *instead* of the descriptor above.
//|     See page 61 in the `Device Class Definition for Human Interface Devices (HID) <https://www.usb.org/sites/default/files/hid1_11.pdf>`_.
//|     """
//|
//|     CONSUMER_CONTROL: Device
//|     """Consumer Control device supporting sent values from 1-652, with no rollover.
//|     The `Device` values are shown below.
//|
//|     .. code-block::
//|
//|       CONSUMER_CONTROL = Device(descriptor=<see below>, usage_page=0x0C, usage=0x01,
//|                                 in_report_length=4, out_report_length=0, report_id_index=7,
//|                                 boot=False)
//|
//|       Consumer Control report descriptor:
//|
//|        0x05, 0x0C,        # Usage Page (Consumer)
//|        0x09, 0x01,        # Usage (Consumer Control)
//|        0xA1, 0x01,        # Collection (Application)
//|        0x85, 0x--,        #   Report ID  [set at runtime, will be omitted if sole device]
//|        0x75, 0x10,        #   Report Size (16)
//|        0x95, 0x01,        #   Report Count (1)
//|        0x15, 0x01,        #   Logical Minimum (1)
//|        0x26, 0x8C, 0x02,  #   Logical Maximum (652)
//|        0x19, 0x01,        #   Usage Minimum (Consumer Control)
//|        0x2A, 0x8C, 0x02,  #   Usage Maximum (AC Send)
//|        0x81, 0x00,        #   Input (Data,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
//|        0xC0,              # End Collection
//|      """
//|

STATIC mp_obj_t usb_hid_device_make_new(const mp_obj_type_t *type, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    usb_hid_device_obj_t *self = m_new_obj(usb_hid_device_obj_t);
    self->base.type = &usb_hid_device_type;
    enum { ARG_report_descriptor, ARG_usage_page, ARG_usage, ARG_in_report_length, ARG_out_report_length, ARG_report_id_index, ARG_boot };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_report_descriptor, MP_ARG_KW_ONLY | MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_usage_page, MP_ARG_KW_ONLY | MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_usage, MP_ARG_KW_ONLY | MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_in_report_length, MP_ARG_KW_ONLY | MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_out_report_length, MP_ARG_KW_ONLY | MP_ARG_OBJ, {.u_int = 0 } },
        { MP_QSTR_report_id_index, MP_ARG_KW_ONLY | MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = mp_const_none } },
        { MP_QSTR_boot, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_report_descriptor].u_obj, &bufinfo, MP_BUFFER_READ);
    mp_obj_t descriptor = mp_obj_new_bytearray(bufinfo.len, bufinfo.buf);

    const mp_int_t usage_page_arg = args[ARG_usage_page].u_int;
    if (usage_page_arg <= 0 || usage_page_arg > 255) {
        mp_raise_ValueError_varg(translate("%q must be 1-255"), MP_QSTR_usage_page);
    }
    const uint8_t usage_page = usage_page_arg;

    const mp_int_t usage_arg = args[ARG_usage].u_int;
    if (usage_arg <= 0 || usage_arg > 255) {
        mp_raise_ValueError_varg(translate("%q must be 1-255"), MP_QSTR_usage);
    }
    const uint8_t usage = usage_arg;

    const mp_int_t in_report_length_arg = args[ARG_in_report_length].u_int;
    if (in_report_length_arg <= 0 || in_report_length_arg > 255) {
        mp_raise_ValueError_varg(translate("%q must be 1-255"), MP_QSTR_in_report_length);
    }
    const uint8_t in_report_length = in_report_length_arg;

    const mp_int_t out_report_length_arg = args[ARG_out_report_length].u_int;
    if (out_report_length_arg <= 0 || out_report_length_arg > 255) {
        mp_raise_ValueError_varg(translate("%q must be 1-255"), MP_QSTR_out_report_length);
    }
    const uint8_t out_report_length = out_report_length_arg;

    const mp_obj_t report_id_index_arg = args[ARG_report_id_index].u_obj;
    uint8_t report_id_index = 0;
    if (report_id_index_arg != mp_const_none) {
        const mp_int_t report_id_index_int = mp_obj_int_get_checked(report_id_index_arg);
        if (report_id_index_int <= 0 || report_id_index_int > 255) {
            mp_raise_ValueError_varg(translate("%q must be None or 1-255"), MP_QSTR_report_id_index);
        }
        report_id_index = report_id_index_int;
    }

    const bool boot = args[ARG_boot].u_bool;
    if (boot && !(usage_page == 1 && (usage == 6 || usage == 2))) {
        mp_raise_ValueError(translate("device must be a keyboard or mouse if boot is True"));
    }
    common_hal_usb_hid_device_construct(self, descriptor, usage_page, usage,
        in_report_length, out_report_length, report_id_index,
        boot);
    return (mp_obj_t)self;
}


//|     def send_report(self, buf: ReadableBuffer) -> None:
//|         """Send a HID report."""
//|         ...
//|
STATIC mp_obj_t usb_hid_device_send_report(mp_obj_t self_in, mp_obj_t buffer) {
    usb_hid_device_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buffer, &bufinfo, MP_BUFFER_READ);

    common_hal_usb_hid_device_send_report(self, ((uint8_t *)bufinfo.buf), bufinfo.len);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(usb_hid_device_send_report_obj, usb_hid_device_send_report);

//|     interface_number: int
//|     """The interface number in which this device is present."""
//|
STATIC mp_obj_t usb_hid_device_obj_get_interface_number(mp_obj_t self_in) {
    usb_hid_device_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->out_report_buffer == 0) {
        return mp_const_none;
    }
    return MP_OBJ_NEW_SMALL_INT(common_hal_usb_hid_device_get_interface_number(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(usb_hid_device_get_interface_number_obj, usb_hid_device_obj_get_interface_number);

const mp_obj_property_t usb_hid_device_interface_number_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&usb_hid_device_get_interface_number_obj,
              MP_ROM_NONE,
              MP_ROM_NONE},
};

//|     last_received_report: bytes
//|     """The HID OUT report as a `bytes`. (read-only). `None` if nothing received."""
//|
STATIC mp_obj_t usb_hid_device_obj_get_last_received_report(mp_obj_t self_in) {
    usb_hid_device_obj_t *self = MP_OBJ_TO_PTR(self_in);
    if (self->out_report_buffer == 0) {
        return mp_const_none;
    }
    return mp_obj_new_bytes(self->out_report_buffer, self->out_report_length);
}
MP_DEFINE_CONST_FUN_OBJ_1(usb_hid_device_get_last_received_report_obj, usb_hid_device_obj_get_last_received_report);

const mp_obj_property_t usb_hid_device_last_received_report_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&usb_hid_device_get_last_received_report_obj,
              MP_ROM_NONE,
              MP_ROM_NONE},
};

//|     usage_page: int
//|     """The usage page of the device as an `int`. Can be thought of a category. (read-only)"""
//|
STATIC mp_obj_t usb_hid_device_obj_get_usage_page(mp_obj_t self_in) {
    usb_hid_device_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_usb_hid_device_get_usage_page(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(usb_hid_device_get_usage_page_obj, usb_hid_device_obj_get_usage_page);

const mp_obj_property_t usb_hid_device_usage_page_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&usb_hid_device_get_usage_page_obj,
              MP_ROM_NONE,
              MP_ROM_NONE},
};

//|     usage: int
//|     """The functionality of the device as an int. (read-only)
//|
//|     For example, Keyboard is 0x06 within the generic desktop usage page 0x01.
//|     Mouse is 0x02 within the same usage page."""
//|
STATIC mp_obj_t usb_hid_device_obj_get_usage(mp_obj_t self_in) {
    usb_hid_device_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_usb_hid_device_get_usage(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(usb_hid_device_get_usage_obj,
    usb_hid_device_obj_get_usage);

const mp_obj_property_t usb_hid_device_usage_obj = {
    .base.type = &mp_type_property,
    .proxy = {(mp_obj_t)&usb_hid_device_get_usage_obj,
              MP_ROM_NONE,
              MP_ROM_NONE},
};

STATIC const mp_rom_map_elem_t usb_hid_device_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_send_report),          MP_ROM_PTR(&usb_hid_device_send_report_obj) },
    { MP_ROM_QSTR(MP_QSTR_interface_number),     MP_ROM_PTR(&usb_hid_device_interface_number_obj) },
    { MP_ROM_QSTR(MP_QSTR_last_received_report), MP_ROM_PTR(&usb_hid_device_last_received_report_obj) },
    { MP_ROM_QSTR(MP_QSTR_usage_page),           MP_ROM_PTR(&usb_hid_device_usage_page_obj) },
    { MP_ROM_QSTR(MP_QSTR_usage),                MP_ROM_PTR(&usb_hid_device_usage_obj) },
    { MP_ROM_QSTR(MP_QSTR_KEYBOARD),             MP_ROM_PTR(&usb_hid_device_keyboard_obj) },
    { MP_ROM_QSTR(MP_QSTR_MOUSE),                MP_ROM_PTR(&usb_hid_device_mouse_obj) },
    { MP_ROM_QSTR(MP_QSTR_CONSUMER_CONTROL),     MP_ROM_PTR(&usb_hid_device_consumer_control_obj) },
};

STATIC MP_DEFINE_CONST_DICT(usb_hid_device_locals_dict, usb_hid_device_locals_dict_table);

const mp_obj_type_t usb_hid_device_type = {
    { &mp_type_type },
    .name = MP_QSTR_Device,
    .make_new = usb_hid_device_make_new,
    .locals_dict = (mp_obj_t)&usb_hid_device_locals_dict,
};
