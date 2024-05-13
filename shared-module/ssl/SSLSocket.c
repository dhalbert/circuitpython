/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Linaro Ltd.
 * Copyright (c) 2019 Paul Sokolovsky
 * Copyright (c) 2022 Jeff Epler for Adafruit Industries
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

#include "shared-bindings/ssl/__init__.h"
#include "shared-bindings/ssl/SSLSocket.h"
#include "shared-bindings/ssl/SSLContext.h"

#include "shared/runtime/interrupt_char.h"
#include "shared/netutils/netutils.h"
#include "py/mphal.h"
#include "py/objstr.h"
#include "py/runtime.h"
#include "py/stream.h"
#include "supervisor/shared/tick.h"

#include "shared-bindings/socketpool/enum.h"

#ifdef MBEDTLS_DEBUG_C
#include "mbedtls/debug.h"
STATIC void mbedtls_debug(void *ctx, int level, const char *file, int line, const char *str) {
    (void)ctx;
    (void)level;
    mp_printf(&mp_plat_print, "DBG:%s:%04d: %s\n", file, line, str);
}
#define DEBUG_PRINT(fmt, ...) mp_printf(&mp_plat_print, "DBG:%s:%04d: " fmt "\n", __FILE__, __LINE__,##__VA_ARGS__)
#else
#define DEBUG_PRINT(...) do {} while (0)
#endif

// Because ssl_socket_send and ssl_socket_recv_into are callbacks from mbedtls code,
// it is not OK to exit them by raising an exception (nlr_jump'ing through
// foreign code is not permitted). Instead, preserve the error number of any OSError
// and turn anything else into -MP_EINVAL.
STATIC int call_method_errno(size_t n_args, const mp_obj_t *args) {
    nlr_buf_t nlr;
    mp_int_t result = -MP_EINVAL;
    if (nlr_push(&nlr) == 0) {
        mp_obj_t obj_result = mp_call_method_n_kw(n_args, 0, args);
        result = (obj_result == mp_const_none) ? 0 : mp_obj_get_int(obj_result);
        nlr_pop();
        return result;
    } else {
        mp_obj_t exc = MP_OBJ_FROM_PTR(nlr.ret_val);
        if (nlr_push(&nlr) == 0) {
            result = -mp_obj_get_int(mp_load_attr(exc, MP_QSTR_errno));
            nlr_pop();
        }
    }
    return result;
}

static int ssl_socket_send(ssl_sslsocket_obj_t *self, const byte *buf, size_t len) {
    mp_obj_array_t mv;
    mp_obj_memoryview_init(&mv, 'B', 0, len, (void *)buf);

    self->send_args[2] = MP_OBJ_FROM_PTR(&mv);
    return call_method_errno(1, self->send_args);
}

static int ssl_socket_recv_into(ssl_sslsocket_obj_t *self, byte *buf, size_t len) {
    mp_obj_array_t mv;
    mp_obj_memoryview_init(&mv, 'B' | MP_OBJ_ARRAY_TYPECODE_FLAG_RW, 0, len, buf);

    self->recv_into_args[2] = MP_OBJ_FROM_PTR(&mv);
    return call_method_errno(1, self->recv_into_args);
}

static void ssl_socket_connect(ssl_sslsocket_obj_t *self, mp_obj_t addr_in) {
    self->connect_args[2] = addr_in;
    mp_call_method_n_kw(1, 0, self->connect_args);
}

static void ssl_socket_bind(ssl_sslsocket_obj_t *self, mp_obj_t addr_in) {
    self->bind_args[2] = addr_in;
    mp_call_method_n_kw(1, 0, self->bind_args);
}

static void ssl_socket_close(ssl_sslsocket_obj_t *self) {
    // swallow any exception raised by the underlying close method.
    // This is not ideal. However, it avoids printing "MemoryError:"
    // when attempting to close a userspace socket object during gc_sweep_all
    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_call_method_n_kw(0, 0, self->close_args);
        nlr_pop();
    } else {
        nlr_pop();
    }
}

static void ssl_socket_setsockopt(ssl_sslsocket_obj_t *self, mp_obj_t level_obj, mp_obj_t opt_obj, mp_obj_t optval_obj) {
    self->setsockopt_args[2] = level_obj;
    self->setsockopt_args[3] = opt_obj;
    self->setsockopt_args[4] = optval_obj;
    mp_call_method_n_kw(3, 0, self->setsockopt_args);
}

static void ssl_socket_settimeout(ssl_sslsocket_obj_t *self, mp_obj_t timeout_obj) {
    self->settimeout_args[2] = timeout_obj;
    mp_call_method_n_kw(1, 0, self->settimeout_args);
}

static void ssl_socket_listen(ssl_sslsocket_obj_t *self, mp_int_t backlog) {
    self->listen_args[2] = MP_OBJ_NEW_SMALL_INT(backlog);
    mp_call_method_n_kw(1, 0, self->listen_args);
}

static mp_obj_t ssl_socket_accept(ssl_sslsocket_obj_t *self) {
    return mp_call_method_n_kw(0, 0, self->accept_args);
}

int ssl_socket_mbedtls_ssl_send(void *ctx, const byte *buf, size_t len) {
    ssl_sslsocket_obj_t *self = (ssl_sslsocket_obj_t *)ctx;

    mp_int_t out_sz = ssl_socket_send(self, buf, len);
    DEBUG_PRINT("socket_send() -> %d", out_sz);
    if (out_sz < 0) {
        int err = -out_sz;
        DEBUG_PRINT("sock_stream->write() -> %d nonblocking? %d", out_sz, mp_is_nonblocking_error(err));
        if (mp_is_nonblocking_error(err)) {
            return MBEDTLS_ERR_SSL_WANT_WRITE;
        }
    }
    return out_sz;
}

int ssl_socket_mbedtls_ssl_recv(void *ctx, byte *buf, size_t len) {
    ssl_sslsocket_obj_t *self = (ssl_sslsocket_obj_t *)ctx;

    mp_int_t out_sz = ssl_socket_recv_into(self, buf, len);
    DEBUG_PRINT("socket_recv() -> %d", out_sz);
    if (out_sz < 0) {
        int err = -out_sz;
        if (mp_is_nonblocking_error(err)) {
            return MBEDTLS_ERR_SSL_WANT_READ;
        }
    }
    return out_sz;
}


mp_uint_t common_hal_ssl_sslsocket_recv_into(ssl_sslsocket_obj_t *self, uint8_t *buf, uint32_t len) {
    int ret = mbedtls_ssl_read(&self->ssl, buf, len);
    DEBUG_PRINT("recv_into mbedtls_ssl_read() -> %d\n", ret);
    if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
        DEBUG_PRINT("returning %d\n", 0);
        // end of stream
        return 0;
    }
    if (ret >= 0) {
        DEBUG_PRINT("returning %d\n", ret);
        return ret;
    }
    DEBUG_PRINT("raising errno [error case] %d\n", ret);
    mbedtls_raise_error(ret);
}

mp_uint_t common_hal_ssl_sslsocket_send(ssl_sslsocket_obj_t *self, const uint8_t *buf, uint32_t len) {
    int ret = mbedtls_ssl_write(&self->ssl, buf, len);
    DEBUG_PRINT("send mbedtls_ssl_write() -> %d\n", ret);
    if (ret >= 0) {
        DEBUG_PRINT("returning %d\n", ret);
        return ret;
    }
    DEBUG_PRINT("raising errno [error case] %d\n", ret);
    mbedtls_raise_error(ret);
}

void common_hal_ssl_sslsocket_bind(ssl_sslsocket_obj_t *self, mp_obj_t addr_in) {
    ssl_socket_bind(self, addr_in);
}

void common_hal_ssl_sslsocket_close(ssl_sslsocket_obj_t *self) {
    if (self->closed) {
        return;
    }
    self->closed = true;
    ssl_socket_close(self);
    mbedtls_ssl_free(&self->ssl);
}

STATIC void do_handshake(ssl_sslsocket_obj_t *self) {
    int ret;
    while ((ret = mbedtls_ssl_handshake(&self->ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            goto cleanup;
        }
        RUN_BACKGROUND_TASKS;
        if (MP_STATE_THREAD(mp_pending_exception) != MP_OBJ_NULL) {
            mp_handle_pending(true);
        }
        mp_hal_delay_ms(1);
    }

    return;

cleanup:
    self->closed = true;

    mbedtls_ssl_free(&self->ssl);

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

void common_hal_ssl_sslsocket_connect(ssl_sslsocket_obj_t *self, mp_obj_t addr_in) {
    ssl_socket_connect(self, addr_in);
    do_handshake(self);
}

bool common_hal_ssl_sslsocket_get_closed(ssl_sslsocket_obj_t *self) {
    return self->closed;
}

bool common_hal_ssl_sslsocket_get_connected(ssl_sslsocket_obj_t *self) {
    return !self->closed;
}

void common_hal_ssl_sslsocket_listen(ssl_sslsocket_obj_t *self, int backlog) {
    return ssl_socket_listen(self, backlog);
}

mp_obj_t common_hal_ssl_sslsocket_accept(ssl_sslsocket_obj_t *self) {
    mp_obj_t accepted = ssl_socket_accept(self);
    mp_obj_t sock = mp_obj_subscr(accepted, MP_OBJ_NEW_SMALL_INT(0), MP_OBJ_SENTINEL);
    ssl_sslsocket_obj_t *sslsock = common_hal_ssl_sslcontext_wrap_socket(self->ssl_context, sock, true, NULL);
    do_handshake(sslsock);
    mp_obj_t peer = mp_obj_subscr(accepted, MP_OBJ_NEW_SMALL_INT(1), MP_OBJ_SENTINEL);
    mp_obj_t tuple_contents[2];
    tuple_contents[0] = MP_OBJ_FROM_PTR(sslsock);
    tuple_contents[1] = peer;
    return mp_obj_new_tuple(2, tuple_contents);
}

void common_hal_ssl_sslsocket_setsockopt(ssl_sslsocket_obj_t *self, mp_obj_t level_obj, mp_obj_t optname_obj, mp_obj_t optval_obj) {
    ssl_socket_setsockopt(self, level_obj, optname_obj, optval_obj);
}

void common_hal_ssl_sslsocket_settimeout(ssl_sslsocket_obj_t *self, mp_obj_t timeout_obj) {
    ssl_socket_settimeout(self, timeout_obj);
}
