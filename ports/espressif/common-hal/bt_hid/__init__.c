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

#include <string.h>

#include "components/bt/host/bluedroid/api/include/api/esp_bt_device.h"
#include "components/bt/host/bluedroid/api/include/api/esp_gap_bt_api.h"
#include "components/esp_hid/include/esp_hidd.h"

#include "py/gc.h"
#include "py/mphal.h"
#include "py/mpstate.h"
#include "py/runtime.h"
#include "shared-bindings/bt_hid/__init__.h"
#include "shared-bindings/bt_hid/Device.h"
#include "supervisor/port.h"
#include "supervisor/usb.h"

// Which boot device is available? 0: no boot devices, 1: boot keyboard, 2: boot mouse.
// This value is set by bt_hid.enable(), and used to build the HID interface descriptor.
// The value is remembered here from boot.py to code.py.
static uint8_t hid_boot_device;

// Whether a boot device was requested by a SET_PROTOCOL request from the host.
static bool hid_boot_device_requested;

static mp_obj_tuple_t default_bt_hid_devices_tuple = {
    .base = {
        .type = &mp_type_tuple,
    },
    .len = 3,
    .items = {
        MP_OBJ_FROM_PTR(&bt_hid_device_keyboard_obj),
        MP_OBJ_FROM_PTR(&bt_hid_device_mouse_obj),
        MP_OBJ_FROM_PTR(&bt_hid_device_consumer_control_obj),
    },
};

// These describe the standard descriptors used for boot keyboard and mouse, which don't use report IDs.
// When the host requests a boot device, replace whatever HID devices were enabled with a tuple
// containing just one of these, since the host is uninterested in other devices.
// The driver code will then use the proper report length and send_report() will not send a report ID.
// static const bt_hid_device_obj_t boot_keyboard_obj = {
//     .base = {
//         .type = &bt_hid_device_type,
//     },
//     .report_descriptor = NULL,
//     .report_descriptor_length = 0,
//     .usage_page = 0x01,
//     .usage = 0x06,
//     .num_report_ids = 1,
//     .report_ids = { 0, },
//     .in_report_lengths = { 8, },
//     .out_report_lengths = { 1, },
// };

// static const bt_hid_device_obj_t boot_mouse_obj = {
//     .base = {
//         .type = &bt_hid_device_type,
//     },
//     .report_descriptor = NULL,
//     .report_descriptor_length = 0,
//     .usage_page = 0x01,
//     .usage = 0x02,
//     .num_report_ids = 1,
//     .report_ids = { 0, },
//     .in_report_lengths = { 4, },
//     .out_report_lengths = { 0, },
// };

// typedef struct
// {
//     TaskHandle_t task_hdl;
//     esp_hidd_dev_t *hid_dev;
//     uint8_t protocol_mode;
//     uint8_t *buffer;
// } local_param_t;

// static local_param_t s_bt_hid_param = {0};

// Set by esp_hidd_dev_init().
static esp_hidd_dev_t *hid_dev;

static void bt_hidd_event_callback(void *handler_args, esp_event_base_t base, int32_t id, void *event_data) {
    esp_hidd_event_t event = (esp_hidd_event_t)id;
    esp_hidd_event_data_t *param = (esp_hidd_event_data_t *)event_data;

    switch (event) {
        case ESP_HIDD_START_EVENT: {
            if (param->start.status == ESP_OK) {
                esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            } else {
                // START failed
            }
            break;
        }
        case ESP_HIDD_CONNECT_EVENT: {
            if (param->connect.status == ESP_OK) {
                esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
            } else {
                // CONNECT failed
            }
            break;
        }
        case ESP_HIDD_PROTOCOL_MODE_EVENT: {
            // if (param->protocol_mode.protocol_mode > 0) {
            //     // Will be 1 (keyboard) or 2 (mouse).

            //     memcpy(&hid_devices[0],
            //         hid_boot_device == 1 ? &boot_keyboard_obj : &boot_mouse_obj,
            //         sizeof(bt_hid_device_obj_t));
            //     num_hid_devices = 1;
            // }
            break;

            // ESP_LOGI(TAG, "PROTOCOL MODE[%u]: %s", param->protocol_mode.map_index, param->protocol_mode.protocol_mode ? "REPORT" : "BOOT");
            break;
        }
        case ESP_HIDD_OUTPUT_EVENT: {
            // TODO OUTPUT EVENT
            // ESP_LOGI(TAG, "OUTPUT[%u]: %8s ID: %2u, Len: %d, Data:", param->output.map_index, esp_hid_usage_str(param->output.usage), param->output.report_id, param->output.length);
            // ESP_LOG_BUFFER_HEX(TAG, param->output.data, param->output.length);
            break;
        }
        case ESP_HIDD_FEATURE_EVENT: {
            // TODO FEATURE EVENT
            // ESP_LOGI(TAG, "FEATURE[%u]: %8s ID: %2u, Len: %d, Data:", param->feature.map_index, esp_hid_usage_str(param->feature.usage), param->feature.report_id, param->feature.length);
            // ESP_LOG_BUFFER_HEX(TAG, param->feature.data, param->feature.length);
            break;
        }
        case ESP_HIDD_DISCONNECT_EVENT: {
            if (param->disconnect.status == ESP_OK) {
                esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            } else {
                // DISCONNECT failed
            }
            break;
        }
        case ESP_HIDD_STOP_EVENT: {
            // STOP
            break;
        }
        default:
            break;
    }
    return;
}

static bool bt_hid_running;

uint8_t bt_hid_boot_device(void) {
    return hid_boot_device;
}

void bt_hid_reset(void) {
    bt_hid_running = false;
}

// Returns 1 or 2 if host requested a boot device and boot protocol was enabled in the interface descriptor.
uint8_t common_hal_bt_hid_get_boot_device(void) {
    return hid_boot_device_requested ? hid_boot_device : 0;
}

bool common_hal_bt_hid_stop(void) {
    MP_STATE_VM(bt_hid_devices_tuple) = mp_const_empty_tuple;
    bt_hid_set_devices(MP_STATE_VM(bt_hid_devices_tuple));

    return esp_hidd_dev_deinit(hid_dev) == ESP_OK;
}

bool common_hal_bt_hid_start(const mp_obj_t devices_in, uint8_t boot_device) {
    mp_obj_t devices_seq = (devices_in == mp_const_none) ? &default_bt_hid_devices_tuple : devices_in;

    const mp_int_t num_devices = MP_OBJ_SMALL_INT_VALUE(mp_obj_len(devices_seq));

    hid_boot_device = boot_device;

    mp_obj_t tuple_items[num_devices];
    esp_hid_raw_report_map_t esp_hid_raw_report_maps[num_devices];

    esp_hid_device_config_t bt_hid_config = {
        .vendor_id = 0x239a,
        .product_id = 0x0001,
        .version = 0x0100,
        .device_name = "Bluetooth Classic HID",
        .manufacturer_name = "CircuitPython",
        .serial_number = "1234567890",
        .report_maps = esp_hid_raw_report_maps,
        .report_maps_len = 1,
    };


    for (mp_int_t i = 0; i < num_devices; i++) {
        // Extract bt_hid.Device objects from the passed-in sequence, by subscripting.
        // devices_seq has already been validated to contain only bt_hid_device_obj_t objects.
        bt_hid_device_obj_t *device =
            MP_OBJ_TO_PTR(mp_obj_subscr(devices_seq, MP_OBJ_NEW_SMALL_INT(i), MP_OBJ_SENTINEL));

        // Save in a tuple for returning to Python.
        tuple_items[i] = device;

        // Also save for passing to ESP-IDF, which will handle making up the HID interface descriptor.
        esp_hid_raw_report_maps[i].data = device->report_descriptor;
        esp_hid_raw_report_maps[i].len = device->report_descriptor_length;

        // Create report buffers on the heap.
        bt_hid_device_create_report_buffers(device);
    }

    // Remember tuple for gc purposes.
    MP_STATE_VM(bt_hid_devices_tuple) = mp_obj_new_tuple(num_devices, tuple_items);
    bt_hid_set_devices(MP_STATE_VM(bt_hid_devices_tuple));

    esp_bt_dev_set_device_name(bt_hid_config.device_name);
    esp_bt_cod_t cod = {
        .major = ESP_BT_COD_MAJOR_DEV_PERIPHERAL,
    };
    esp_bt_gap_set_cod(cod, ESP_BT_SET_COD_MAJOR_MINOR);
    mp_hal_delay_ms(1);
    // esp_hidd_dev_init copies the configs, so bt_hid_config does not need to contain static inof.
    return esp_hidd_dev_init(&bt_hid_config, ESP_HID_TRANSPORT_BT, bt_hidd_event_callback, &hid_dev) == ESP_OK;
}

bool bt_hid_get_device_with_report_id(uint8_t report_id, bt_hid_device_obj_t **device_out, size_t *id_idx_out) {
    const size_t num_devices = MP_STATE_VM(bt_hid_devices_tuple)->len;
    for (uint8_t i = 0; i < num_devices; i++) {
        bt_hid_device_obj_t *device = MP_STATE_VM(bt_hid_devices_tuple)->items[i];
        for (size_t report_id_idx = 0; report_id_idx < device->num_report_ids; report_id_idx++) {
            if (device->report_ids[report_id_idx] == report_id) {
                *device_out = device;
                *id_idx_out = report_id_idx;
                return true;
            }
        }
    }
    return false;
}

// This tuple is store in bt_hid.devices.
MP_REGISTER_ROOT_POINTER(mp_obj_tuple_t * bt_hid_devices_tuple);
