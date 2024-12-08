// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2024 by Dan Halbert for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include <string.h>

#include "py/runtime.h"
#include "shared-bindings/cryptio/__init__.h"
#include "lib/mbedtls/include/psa/crypto.h"

void common_hal_cryptio_init(void) {
    status = psa_crypto_init();
    if (status != PSA_SUCCESS) {
        mp_raise_Runtime_Error_varg(MP_ERROR_TEXT("%q init failed"), MP_QSTR_cryptio);
    }
}
