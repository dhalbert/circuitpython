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


#include "py/runtime.h"
#include "shared-bindings/keypad/KeyStates.h"

keypad_keystates_obj_t *common_hal_keypad_new_keystates(keypad_scan_obj_t *scan, keypad_state_t state) {
    keypad_keystates_obj_t *self = m_new_obj(keypad_keystates_obj_t);
    self->base.type = &keypad_keystates_type;
    self->scan = scan;
    self->state = state;
    self->last_key_num_match = -1;
    return self;
}

// Returns sentinel value -1 when there are no more matches.
// Otherwise returns the key_num of the next key that matches.
mp_obj_t common_hal_keypad_keystates_next(keypad_keystates_obj_t *self) {
    size_t len = common_hal_keypad_scan_length(self->scan);

    while (++self->last_key_num_match < len) {
        if (common_hal_keypad_key_has_state(scan, self->last_key_num_match, self->state)) {
            return self->last_key_num_match;
        }
    }
    return -1;
}
