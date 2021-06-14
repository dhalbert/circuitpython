/*
 * This file is part of the Micro Python project, http://micropython.org/
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

#ifndef MICROPY_INCLUDED_SHARED_BINDINGS_KEYPAD_KEYMATRIX_H
#define MICROPY_INCLUDED_SHARED_BINDINGS_KEYPAD_KEYMATRIX_H

#include "py/objlist.h"
#include "shared-module/keypad/Keys.h"

extern const mp_obj_type_t keypad_keymatrix_type;

void common_hal_keypad_keymatrix_construct(keypad_keymatrix_obj_t *self, mp_uint_t num_row_pins, mcu_pin_obj_t *row_pins[], mp_uint_t num_col_pins, mcu_pin_obj_t *col_pins[]);
void common_hal_keypad_keymatrix_keys_with_state(keypad_keymatrix_obj_t *self, mp_int_t state, mp_obj_list_t *into);
size_t common_hal_keypad_keymatrix_row_length(keypad_keymatrix_obj_t *self);
size_t common_hal_keypad_keymatrix_col_length(keypad_keymatrix_obj_t *self);
mp_int_t common_hal_keypad_keymatrix_key_num(mp_int_t row, mp_int_t col);
bool common_hal_keypad_keymatrix_scan(keypad_keymatrix_obj_t *self);
mp_int_t common_hal_keypad_keymatrix_state(keypad_keymatrix_obj_t *self, mp_uint_t key_num);

#endif  // MICROPY_INCLUDED_SHARED_BINDINGS_KEYPAD_KEYMATRIX_H
