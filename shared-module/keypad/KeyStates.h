/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Dan Halbert for Adafruit Industries
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

#ifndef MICROPY_INCLUDED_SHARED_MODULE_KEYPAD_KEYSTATES_H
#define MICROPY_INCLUDED_SHARED_MODULE_KEYPAD_KEYSTATES_H

#include <stdint.h>

#include "shared-module/keypad/KeyStates.h"
#include "shared-module/keypad/Scan.h"


typedef struct {
    mp_obj_base_t base;
    keypad_scan_obj_t *scan;
    keypad_state_t state;
    mp_int_t last_key_num_match;
} keypad_keystates_obj_t;

keypad_keystates_obj_t *common_hal_keypad_new_keystates(keypad_scan_obj_t *scan, keypad_state_t state);
mp_obj_t common_hal_keypad_keystates_next(keypad_keystates_obj_t *self)

#endif // MICROPY_INCLUDED_SHARED_MODULE_KEYPAD_KEYSTATES_H
