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

#include "shared-bindings/keypad/Scan.h"
#include "shared-bindings/keypad/State.h"

void common_hal_keypad_scan_construct(keypad_scan_obj_t *self, size_t num_keys) {
    self->num_keys = num_keys;
    for (size_t key_num = 0; key_num < num_keys; key_num++) {
        self->states[key_num] = STATE_STILL_RELEASED;
    }
}

bool common_hal_keypad_scan_key_has_state(keypad_keys_obj_t *self, mp_uint_t key_num, keypad_state_t state) {
    const keypad_state_t actual_state = self->state[key_num];
    switch (state) {
        case STATE_PRESSED:
            return actual_state == STATE_JUST_PRESSED || actual_state == STATE_STILL_PRESSED;
        case STATE_RELEASED:
            return actual_state == STATE_JUST_RELEASED || actual_state == STATE_STILL_RELEASED;
        default:
            return actual_state == state;
    }
}

key_state_t common_hal_keypad_scan_key_state(keypad_keys_obj_t *self, mp_uint_t key_num) {
    return self->states[key_num];
}

keypad_keystates_obj_t *common_hal_keypad_scan_keys_with_state(keypad_keys_obj_t *self, keypad_state_t state) {
    keypad_keystates_obj_t *keystates = m_new_obj(keypad_keystates_obj_t);
    keystates->base.type = &keypad_keystates_type;

    return common_hal_keypad_keystates_construct(keystates, state);
}

size_t common_hal_keypad_scan_num_keys(keypad_keys_obj_t *self) {
    return self->num_keys;
}

// states has been validated to be of the correct length.
void common_hal_keypad_scan_set_key_states(keypad_keys_obj_t *self, keypad_state_t states[]) {
    for (size_t i = 0; i < self->num_keys; i++) {
        self->states[i] = states[i];
    }
}
