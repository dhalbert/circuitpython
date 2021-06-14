/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Dan Halbert for Adafruit Industries
 * Copyright (c) 2018 Artur Pacholec
 * Copyright (c) 2017 Glenn Ruben Bakke
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

#include "py/objproperty.h"
#include "py/runtime.h"
#include "shared-bindings/_bleio/ScanResults.h"

//| class KeyStates:
//|     """An iterator that returns `int` key numbers with a given state.
//|     Returned by `Scan.keys_with_state()`. It has no user-visible constructor."""
//|
STATIC mp_obj_t keystates_iternext(mp_obj_t self_in) {
    mp_check_self(mp_obj_is_type(self_in, &keypad_keystates_type));
    keypad_keystates_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t key_num = common_hal_keypad_keystates_next(self);
    if (key_num != -1) {
        return MP_OBJ_NEW_SMALL_INT(key_num);
    }
    return MP_OBJ_STOP_ITERATION;
}

//|     def __init__(self) -> None:
//|         """Cannot be instantiated directly. Use `keypad.Scan.keys_with_state()`."""
//|         ...
//|
//|     def __iter__(self) -> Iterator[int]:
//|         """Returns itself since it is the iterator."""
//|         ...
//|
//|     def __next__(self) -> int:
//|         """Returns the next key number that matches the given state.
//|         active. Raises `StopIteration` if scanning is finished and no other results are available."""
//|         ...
//|

const mp_obj_type_t keypad_keystates_type = {
    { &mp_type_type },
    .name = MP_QSTR_KeyStates,
    .getiter = mp_identity_getiter,
    .iternext = keystates_iternext,
};
