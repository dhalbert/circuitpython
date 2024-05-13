/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Scott Shawcroft for Adafruit Industries
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

#include "shared-bindings/socketpool/enum.h"
#include "shared-bindings/ssl/__init__.h"
#include "shared-bindings/ssl/SSLContext.h"
#include "shared-bindings/ssl/SSLSocket.h"

#include "py/runtime.h"
#include "py/stream.h"

#include "mbedtls/version.h"

#if MBEDTLS_VERSION_MAJOR >= 3
#include "shared-bindings/os/__init__.h"

static int urandom_adapter(void *unused, unsigned char *buf, size_t n) {
    int result = common_hal_os_urandom(buf, n);
    if (result) {
        return 0;
    }
    return MBEDTLS_ERR_SSL_INTERNAL_ERROR;
}
#endif

void common_hal_ssl_sslcontext_construct(ssl_sslcontext_obj_t *self) {
    common_hal_ssl_sslcontext_set_default_verify_paths(self);

    // Set up as a server socket.
    mbedtls_ssl_config_init(&self->conf);
    mbedtls_x509_crt_init(&self->cacert);
    mbedtls_x509_crt_init(&self->cert);
    mbedtls_pk_init(&self->pkey);
    mbedtls_ctr_drbg_init(&self->ctr_drbg);
    mbedtls_entropy_init(&self->entropy);

    #ifdef MBEDTLS_DEBUG_C
    // Debug level (0-4) 1=warning, 2=info, 3=debug, 4=verbose
    mbedtls_debug_set_threshold(3);
    #endif

    // Whenever the PSA interface is used (if MBEDTLS_PSA_CRYPTO), psa_crypto_init() needs to be called before any TLS related operations.
    // TLSv1.3 depends on the PSA interface, TLSv1.2 only uses the PSA stack if MBEDTLS_USE_PSA_CRYPTO is defined.
    #if defined(MBEDTLS_SSL_PROTO_TLS1_3) || defined(MBEDTLS_USE_PSA_CRYPTO)
    psa_crypto_init();
    #endif

    const byte seed[] = "upy";
    int ret = mbedtls_ctr_drbg_seed(&self->ctr_drbg, mbedtls_entropy_func, &self->entropy, seed, sizeof(seed));
    if (ret != 0) {
        goto cleanup;
    }

    ret = mbedtls_ssl_config_defaults(&self->conf,
        MBEDTLS_SSL_IS_CLIENT,
//        server_side ? MBEDTLS_SSL_IS_SERVER : MBEDTLS_SSL_IS_CLIENT,
        MBEDTLS_SSL_TRANSPORT_STREAM,
        MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret == 0) {
        return;
    }

// An error occurred. Clean up allocations and raise an error based on the return code.
cleanup:
    mbedtls_pk_free(&self->pkey);
    mbedtls_x509_crt_free(&self->cert);
    mbedtls_x509_crt_free(&self->cacert);
    mbedtls_ssl_config_free(&self->conf);
    mbedtls_ctr_drbg_free(&self->ctr_drbg);
    mbedtls_entropy_free(&self->entropy);

    mbedtls_raise_error(ret);
}

void common_hal_ssl_sslcontext_load_verify_locations(ssl_sslcontext_obj_t *self,
    const char *cadata) {
    self->crt_bundle_attach = NULL;
    self->use_global_ca_store = false;
    self->cacert_buf = (const unsigned char *)cadata;
    self->cacert_bytes = *cadata ? strlen(cadata) + 1 : 0;
}

void common_hal_ssl_sslcontext_set_default_verify_paths(ssl_sslcontext_obj_t *self) {
    self->crt_bundle_attach = crt_bundle_attach;
    self->use_global_ca_store = true;
    self->cacert_buf = NULL;
    self->cacert_bytes = 0;
}

bool common_hal_ssl_sslcontext_get_check_hostname(ssl_sslcontext_obj_t *self) {
    return self->check_name;
}

void common_hal_ssl_sslcontext_set_check_hostname(ssl_sslcontext_obj_t *self, bool value) {
    self->check_name = value;
}

void common_hal_ssl_sslcontext_load_cert_chain(ssl_sslcontext_obj_t *self, mp_buffer_info_t *cert_buf, mp_buffer_info_t *key_buf) {
    self->cert_buf = *cert_buf;
    self->key_buf = *key_buf;
}

ssl_sslsocket_obj_t *common_hal_ssl_sslcontext_wrap_socket(ssl_sslcontext_obj_t *self,
    mp_obj_t socket, bool server_side, const char *server_hostname) {

    mp_int_t socket_type = mp_obj_get_int(mp_load_attr(socket, MP_QSTR_type));
    if (socket_type != SOCKETPOOL_SOCK_STREAM) {
        mp_raise_RuntimeError(MP_ERROR_TEXT("Invalid socket for TLS"));
    }

    ssl_sslsocket_obj_t *ssl_socket = m_new_obj_with_finaliser(ssl_sslsocket_obj_t);
    ssl_socket->base.type = &ssl_sslsocket_type;
    ssl_socket->ssl_context = self;
    ssl_socket->sock_obj = socket;

    mp_load_method(socket, MP_QSTR_accept, ssl_socket->accept_args);
    mp_load_method(socket, MP_QSTR_bind, ssl_socket->bind_args);
    mp_load_method(socket, MP_QSTR_close, ssl_socket->close_args);
    mp_load_method(socket, MP_QSTR_connect, ssl_socket->connect_args);
    mp_load_method(socket, MP_QSTR_listen, ssl_socket->listen_args);
    mp_load_method(socket, MP_QSTR_recv_into, ssl_socket->recv_into_args);
    mp_load_method(socket, MP_QSTR_send, ssl_socket->send_args);
    mp_load_method(socket, MP_QSTR_settimeout, ssl_socket->settimeout_args);
    mp_load_method(socket, MP_QSTR_setsockopt, ssl_socket->setsockopt_args);

    mbedtls_ssl_init(&ssl_socket->ssl);

    int ret;

    if (self->crt_bundle_attach != NULL) {
        mbedtls_ssl_conf_authmode(&self->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
        self->crt_bundle_attach(&self->conf);
    } else if (self->cacert_buf && self->cacert_bytes) {
        ret = mbedtls_x509_crt_parse(&self->cacert, self->cacert_buf, self->cacert_bytes);
        if (ret != 0) {
            goto cleanup;
        }
        mbedtls_ssl_conf_authmode(&self->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
        mbedtls_ssl_conf_ca_chain(&self->conf, &self->cacert, NULL);

    } else {
        mbedtls_ssl_conf_authmode(&self->conf, MBEDTLS_SSL_VERIFY_NONE);
    }
    mbedtls_ssl_conf_rng(&self->conf, mbedtls_ctr_drbg_random, &self->ctr_drbg);
    #ifdef MBEDTLS_DEBUG_C
    mbedtls_ssl_conf_dbg(&self->conf, mbedtls_debug, NULL);
    #endif

    ret = mbedtls_ssl_setup(&ssl_socket->ssl, &self->conf);
    if (ret != 0) {
        goto cleanup;
    }

    if (server_hostname != NULL) {
        ret = mbedtls_ssl_set_hostname(&ssl_socket->ssl, server_hostname);
        if (ret != 0) {
            goto cleanup;
        }
    }

    mbedtls_ssl_set_bio(&ssl_socket->ssl, ssl_socket, ssl_socket_mbedtls_ssl_send, ssl_socket_mbedtls_ssl_recv, NULL);

    if (self->cert_buf.buf != NULL) {
        #if MBEDTLS_VERSION_MAJOR >= 3
        ret = mbedtls_pk_parse_key(&self->pkey, self->key_buf.buf, self->key_buf.len + 1, NULL, 0, urandom_adapter, NULL);
        #else
        ret = mbedtls_pk_parse_key(&self->pkey, self->key_buf.buf, self->key_buf.len + 1, NULL, 0);
        #endif
        if (ret != 0) {
            goto cleanup;
        }
        ret = mbedtls_x509_crt_parse(&self->cert, self->cert_buf.buf, self->cert_buf.len + 1);
        if (ret != 0) {
            goto cleanup;
        }

        ret = mbedtls_ssl_conf_own_cert(&self->conf, &self->cert, &self->pkey);
        if (ret != 0) {
            goto cleanup;
        }
    }
    return ssl_socket;

cleanup:
    common_hal_ssl_sslcontext_deinit(self);

    if (ret == MBEDTLS_ERR_SSL_ALLOC_FAILED) {
        mp_raise_type(&mp_type_MemoryError);
    } else if (ret == MBEDTLS_ERR_PK_BAD_INPUT_DATA) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid key"));
    } else if (ret == MBEDTLS_ERR_X509_BAD_INPUT_DATA) {
        mp_raise_ValueError(MP_ERROR_TEXT("invalid cert"));
    } else {
        mbedtls_raise_error(ret);
    }


}

void common_hal_ssl_sslcontext_deinit(ssl_sslcontext_obj_t *self) {
    mbedtls_pk_free(&self->pkey);
    mbedtls_x509_crt_free(&self->cert);
    mbedtls_x509_crt_free(&self->cacert);
    mbedtls_ctr_drbg_free(&self->ctr_drbg);
    mbedtls_entropy_free(&self->entropy);
    mbedtls_ssl_config_free(&self->conf);
}
