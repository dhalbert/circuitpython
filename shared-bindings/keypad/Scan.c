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

#include "py/enum.h"
#include "py/objproperty.h"
#include "shared-bindings/keypad/Scan.h"
#include "shared-bindings/keypad/State.h"
#include "shared-bindings/microcontroller/Pin.h"
#include "py/runtime.h"

//| class Scan:
//|     """A snapshot of a scanned set of keys.
//|     Normally you do not need to instantiate a `Scan` object yourself.
//|     Each instance of a keypad object (`Keys`, `KeypadMatrix`, etc.) holds a single
//|     instance of a Scan.
//|     """
//|         ...

//|     def __init__(self, num_keys: int) -> None:
//|         """
//|         Use this constructor only if you are doing your own scanning and would to
//|         emulate the `keypad` API.
//|
//|         :param int num_keys: The number of keys for which to store states. Must be >= 0.
//|         """

STATIC mp_obj_t keypad_scan_make_new(const mp_obj_type_t *type, size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    keypad_scan_obj_t *self = m_new_obj(keypad_scan_obj_t);
    self->base.type = &keypad_scan_type;
    enum { ARG_num_keys };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_num_keys, MP_ARG_REQUIRED | MP_ARG_INT },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args, pos_args, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_int_t num_keys = args[ARG_num_keys].u_int;
    if (num_keys < 0) {
        mp_raise_ValueError_varg("%q must be >= 0", MP_QSTR_num_keys);
    }

    common_hal_keypad_scan_construct(self, (size_t)num_keys);
    return MP_OBJ_FROM_PTR(self);
}

//|     def key_has_state(self, key_num: int, state: State) -> bool:
//|         """True if key's state matches the given state.
//|
//|         You can specify the inclusive states `State.PRESSED` and `State.RELEASED`.
//|         `State.PRESSED` includes states `State.JUST_PRESSED` and `State.STILL_PRESSED`.
//|         `State.RELEASED` includes `State.JUST_RELEASED` and `State.STILL_RELEASED`.
//|
//|         :rtype: bool"""
//|         """
//|         ...
//|
STATIC mp_obj_t keypad_scan_key_has_state(mp_obj_t self_in, mp_obj_t key_num_in, mp_obj_t state_in) {
    keypad_scan_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (!mp_obj_is_type(state_in, &keypad_state_type)) {
        mp_raise_ValueError_varg(translate("Expected a %q"), keypad_state_type.name);
    }
    cp_enum_obj_t *state = MP_OBJ_TO_PTR(state_in);

    mp_int_t key_num = mp_obj_int_get_checked(key_num_in);
    if (key_num < 0 || (mp_uint_t)key_num >= common_hal_keypad_scan_length(self)) {
        mp_raise_ValueError_varg(translate("%q out of range"), MP_QSTR_key_num);
    }

    return mp_obj_new_bool(common_hal_keypad_scan_key_has_state(self, key_num, state->value));
}
MP_DEFINE_CONST_FUN_OBJ_3(keypad_scan_key_has_state, keypad_scan_key_has_state);

//|         :param int key_num: Key number: corresponds to the sequence of pins
//|         :return: state of key number ``key_num``
//|         :rtype: keypad.State: One of `State.JUST_PRESSED`, `State.STILL_PRESSED`,
//|           `State.JUST_RELEASED`, or `State.STILL_RELEASED`.
//|           The inclusive states `State.PRESSED` and `State.RELEASED` will *not* be returned.
//|         """
//|         ...
//|
STATIC mp_obj_t keypad_scan_key_state(mp_obj_t self_in, mp_obj_t key_num_in) {
    keypad_scan_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t key_num = mp_obj_int_get_checked(key_num_in);
    if (key_num < 0 || (mp_uint_t)key_num >= common_hal_keypad_scan_length(self)) {
        mp_raise_ValueError_varg(translate("%q out of range"), MP_QSTR_key_num);
    }

    return cp_enum_find(&keypad_state_type, common_hal_keypad_scan_key_state(self, (mp_uint_t)key_num));
}
MP_DEFINE_CONST_FUN_OBJ_2(keypad_scan_key_state_obj, keypad_scan_key_state);

//|     def key_state(self, key_num: int) -> keypad.State:
//|         """Return the state for the given ``key_num``.
//|
//|         :param int key_num: Key number: corresponds to the sequence of pins
//|         :return: state of key number ``key_num``
//|         :rtype: keypad.State: One of `State.JUST_PRESSED`, `State.STILL_PRESSED`,
//|           `State.JUST_RELEASED`, or `State.STILL_RELEASED`.
//|           The inclusive states `State.PRESSED` and `State.RELEASED` will *not* be returned.
//|         """
//|         ...
//|
STATIC mp_obj_t keypad_scan_key_state(mp_obj_t self_in, mp_obj_t key_num_in) {
    keypad_scan_obj_t *self = MP_OBJ_TO_PTR(self_in);
    mp_int_t key_num = mp_obj_int_get_checked(key_num_in);
    if (key_num < 0 || (mp_uint_t)key_num >= common_hal_keypad_scan_length(self)) {
        mp_raise_ValueError_varg(translate("%q out of range"), MP_QSTR_key_num);
    }

    return cp_enum_find(&keypad_state_type, common_hal_keypad_scan_key_state(self, (mp_uint_t)key_num));
}
MP_DEFINE_CONST_FUN_OBJ_2(keypad_scan_key_state_obj, keypad_scan_key_state);

//|     def keys_with_state(self, state: State, Iterable[int]) -> None:
//|         """Return an iterable whose values are all the key numbers with the given state.
//|
//|         You can specify the inclusive states `State.PRESSED` and `State.RELEASED`.
//|         `State.PRESSED` includes states `State.JUST_PRESSED` and `State.STILL_PRESSED`.
//|         `State.RELEASED` includes `State.JUST_RELEASED` and `State.STILL_RELEASED`.
//|
//|         :returns: an iterable of ``int`` key numbers
//|         :rtype: iterable"""
//|         """
//|         ...
//|
STATIC mp_obj_t keypad_scan_keys_with_state(mp_obj_t self_in, mp_obj_t state_in) {
    keypad_scan_obj_t *self = MP_OBJ_TO_PTR(self_in);

    if (!mp_obj_is_type(state_in, &keypad_state_type)) {
        mp_raise_ValueError_varg(translate("Expected a %q"), keypad_state_type.name);
    }

    return common_hal_keypad_scan_keys_with_state(self, cp_enum_value(&keypad_state_type, state_in))
}
MP_DEFINE_CONST_FUN_OBJ_2(keypad_scan_keys_with_state_obj, keypad_scan_keys_with_state);

//|     def set_states(self, states: Sequence[State]) -> None:
//|         """
//|         Replace the current states with the give states.
//|         emulate the `keypad` API.
//|
//|         :param Sequence[State] states: The state of each key for the snapshot.
//|         The length of ``states`` must be equal to the number of keys given in the constructor.
//|         """

STATIC mp_obj_t keypad_scan_set_key_states(mp_obj_t self_in, mp_obj_t states) {
    keypad_scan_obj_t *self = MP_OBJ_TO_PTR(self_in);

    const size_t num_states = (size_t)MP_OBJ_SMALL_INT_VALUE(mp_obj_len(states));
    if (num_states != common_hal_keypad_scan_num_keys(self)) {
        mp_raise_ValueError(translate("Wrong number of states"));
    }

    mp_int_t states_array[num_states];
    for (size_t i = 0; i < num_keys; i++) {
        const mp_int_t state = cp_enum_value(&keypad_state_type,
            mp_obj_subscr(pins, MP_OBJ_NEW_SMALL_INT(i), MP_OBJ_SENTINEL));
        if (state == STATE_PRESSED || state == STATE_RELEASED) {
            mp_raise_ValueError_varg("%q must not be %q or %q",
                keypad_state_type.name, MP_QSTR_PRESSED, MP_QSTR_RELEASED);
            states_array[i] = state;
        }

        common_hal_keypad_scan_set_key_states(self, num_keys, states_array);
        return MP_OBJ_FROM_PTR(self);
    }

    STATIC const mp_rom_map_elem_t keypad_scan_locals_dict_table[] = {
        { MP_ROM_QSTR(MP_QSTR_key_has_state),    MP_ROM_PTR(&keypad_scan_key_has_state_obj) },
        { MP_ROM_QSTR(MP_QSTR_key_state),        MP_ROM_PTR(&keypad_scan_state_obj) },
        { MP_ROM_QSTR(MP_QSTR_keys_with_state),  MP_ROM_PTR(&keypad_scan_keys_with_state_obj) },
        { MP_ROM_QSTR(MP_QSTR_set_key_states),   MP_ROM_PTR(&keypad_scan_set_key_states_obj) },
    };

    STATIC MP_DEFINE_CONST_DICT(keypad_scan_locals_dict, keypad_scan_locals_dict_table);

    const mp_obj_type_t keypad_scan_type = {
        { &mp_type_type },
        .name = MP_QSTR_Scan,
        .make_new = keypad_scan_make_new,
        .locals_dict = (mp_obj_t)&keypad_scan_locals_dict,
    };
