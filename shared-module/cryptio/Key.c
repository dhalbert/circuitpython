// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2024 by Dan Halbert for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include <string.h>

#include "py/runtime.h"
#include "shared-bindings/cryptio/Key.h"
#include "lib/mbedtls/include/psa/crypto.h"

void common_hal_cryptio_key_construct(cryptio_key_obj_t *self) {

    // Set up key attributes
    psa_key_attributes_t key_attributes = PSA_KEY_ATTRIBUTES_INIT;

    psa_set_key_usage_flags(&key_attributes, PSA_KEY_USAGE_SIGN_MESSAGE);
    psa_set_key_lifetime(&key_attributes, PSA_KEY_LIFETIME_VOLATILE);
    psa_set_key_algorithm(&key_attributes, PSA_ALG_PURE_EDDSA);
    psa_set_key_type(&key_attributes, PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_TWISTED_EDWARDS));
    psa_set_key_bits(&key_attributes, 255);

    psa_key_id_t privkey_id;
    psa_key_attributes_t privkey_attr = psa_key_attributes_init();

    psa_status_t status;

    status = psa_generate_key(&key_attributes, &m_key_pair_id);
    if (status != PSA_SUCCESS) {
        mp_raise_Runtime_Error_varg(MP_ERROR_TEXT("%q init failed"), MP_QSTR_Key);
    }


    uint8_t public_key[PSA_EXPORT_PUBLIC_KEY_OUTPUT_SIZE(ECC_KEY_TYPE, ECC_KEY_SIZE)] = { 0 };
    size_t pubkey_length;
    status = psa_export_public_key(m_key_pair_id,
        m_pub_key, sizeof(m_pub_key),
        &m_pub_key_len);
    if (status != PSA_SUCCESS) {
        mp_raise_Runtime_Error_varg(MP_ERROR_TEXT("%q init failed"), MP_QSTR_Key);
    }

    psa_reset_key_attributes(&key_attributes);
}

void common_hal_cryptio_init(void) {
    status = psa_crypto_init();
    if (status != PSA_SUCCESS) {
        mp_raise_Runtime_Error_varg(MP_ERROR_TEXT("%q init failed"), MP_QSTR_cryptio);
    }
}
