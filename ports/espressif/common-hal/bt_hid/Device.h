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

#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "py/obj.h"

// The most complicated device currently known of is the head and eye tracker, which requires 5
// report ids.
// https://usb.org/sites/default/files/hutrr74_-_usage_page_for_head_and_eye_trackers_0.pdf
// The default descriptors only use 1, so that is the minimum.
#ifndef CIRCUITPY_BT_HID_MAX_REPORT_IDS_PER_DESCRIPTOR
#define CIRCUITPY_BT_HID_MAX_REPORT_IDS_PER_DESCRIPTOR (6)
#elif CIRCUITPY_BT_HID_MAX_REPORT_IDS_PER_DESCRIPTOR < 1
#error "CIRCUITPY_BT_HID_MAX_REPORT_IDS_PER_DESCRIPTOR must be at least 1"
#endif

typedef struct  {
    mp_obj_base_t base;
    // Python buffer object whose contents are the descriptor.
    const uint8_t *report_descriptor;
    uint8_t *in_report_buffers[CIRCUITPY_BT_HID_MAX_REPORT_IDS_PER_DESCRIPTOR];
    uint8_t *out_report_buffers[CIRCUITPY_BT_HID_MAX_REPORT_IDS_PER_DESCRIPTOR];
    uint8_t out_report_buffers_updated[CIRCUITPY_BT_HID_MAX_REPORT_IDS_PER_DESCRIPTOR];
    uint16_t report_descriptor_length;
    uint8_t report_ids[CIRCUITPY_BT_HID_MAX_REPORT_IDS_PER_DESCRIPTOR];
    uint8_t in_report_lengths[CIRCUITPY_BT_HID_MAX_REPORT_IDS_PER_DESCRIPTOR];
    uint8_t out_report_lengths[CIRCUITPY_BT_HID_MAX_REPORT_IDS_PER_DESCRIPTOR];
    uint16_t usage_page;
    uint16_t usage;
    uint8_t num_report_ids;
} bt_hid_device_obj_t;

extern const bt_hid_device_obj_t bt_hid_device_keyboard_obj;
extern const bt_hid_device_obj_t bt_hid_device_mouse_obj;
extern const bt_hid_device_obj_t bt_hid_device_consumer_control_obj;

void bt_hid_device_create_report_buffers(bt_hid_device_obj_t *self);
