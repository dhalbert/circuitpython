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

#include "common-hal/bt_hid/Device.h"
#include "supervisor/usb.h"

bool bt_hid_enabled(void);
void bt_hid_set_defaults(void);
uint8_t bt_hid_boot_device(void);

size_t bt_hid_add_descriptor(uint8_t *descriptor_buf, descriptor_counts_t *descriptor_counts, uint8_t *current_interface_string, uint16_t report_descriptor_length, uint8_t boot_device);
size_t bt_hid_descriptor_length(void);
size_t bt_hid_report_descriptor_length(void);

void bt_hid_setup_devices(void);
size_t bt_hid_report_descriptor_length(void);
void bt_hid_build_report_descriptor(void);

bool bt_hid_get_device_with_report_id(uint8_t report_id, bt_hid_device_obj_t **device_out, size_t *id_idx_out);

void bt_hid_gc_collect(void);
