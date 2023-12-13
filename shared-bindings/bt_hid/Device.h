/*
 * This file is part of the Micro Python project, http://micropython.org/
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

#include "py/objarray.h"

#include "shared-module/btio/Device.h"

extern const mp_obj_type_t bt_hid_device_type;

void common_hal_bt_hid_device_construct(bt_hid_device_obj_t *self, mp_obj_t report_descriptor, uint16_t usage_page, uint16_t usage, size_t report_ids_count, uint8_t *report_ids, uint8_t *in_report_lengths, uint8_t *out_report_lengths);
void common_hal_bt_hid_device_send_report(bt_hid_device_obj_t *self, uint8_t *report, uint8_t len, uint8_t report_id);
mp_obj_t common_hal_bt_hid_device_get_last_received_report(bt_hid_device_obj_t *self, uint8_t report_id);
uint16_t common_hal_bt_hid_device_get_usage_page(bt_hid_device_obj_t *self);
uint16_t common_hal_bt_hid_device_get_usage(bt_hid_device_obj_t *self);
uint8_t common_hal_bt_hid_device_validate_report_id(bt_hid_device_obj_t *self, mp_int_t report_id);
