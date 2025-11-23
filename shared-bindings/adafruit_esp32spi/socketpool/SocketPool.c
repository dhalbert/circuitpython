// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Dan Halbert for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include "shared-bindings/adafruit_esp32spi/socketpool/SocketPool.h"
#include "shared-bindings/adafruit_esp32spi/ESP_SPIcontrol.h"
#include "shared/runtime/context_manager_helpers.h"
#include "py/objproperty.h"
#include "py/runtime.h"
#include "py/mperrno.h"

//| class SocketPool:
//|     """ESP32 SPI Socket Pool for managing network connections"""
//|
//|     AF_INET: int
//|     """Address family for IPv4"""
//|     SOCK_STREAM: int
//|     """Socket type for TCP"""
//|     SOCK_DGRAM: int
//|     """Socket type for UDP"""
//|
//|     def __init__(self, esp: ESP_SPIcontrol) -> None:
//|         """Create a socket pool using the ESP32 SPI interface.
//|
//|         :param ESP_SPIcontrol esp: The ESP32 SPI control object
//|         """
//|         ...

static mp_obj_t adafruit_esp32spi_socketpool_socketpool_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_esp };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_esp, MP_ARG_REQUIRED | MP_ARG_OBJ },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    adafruit_esp32spi_esp_spicontrol_obj_t *esp =
        mp_arg_validate_type(args[ARG_esp].u_obj, &adafruit_esp32spi_esp_spicontrol_type, MP_QSTR_esp);

    adafruit_esp32spi_socketpool_socketpool_obj_t *self = mp_obj_malloc(adafruit_esp32spi_socketpool_socketpool_obj_t, &adafruit_esp32spi_socketpool_socketpool_type);

    common_hal_adafruit_esp32spi_socketpool_socketpool_construct(self, esp);

    return MP_OBJ_FROM_PTR(self);
}

//|     def socket(self, family: int = AF_INET, type: int = SOCK_STREAM) -> Socket:
//|         """Create a new socket.
//|
//|         :param int family: Socket address family
//|         :param int type: Socket type
//|         :return Socket: A new socket object
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_socketpool_socketpool_socket(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_family, ARG_type };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_family, MP_ARG_INT, {.u_int = ADAFRUIT_ESP32SPI_AF_INET} },
        { MP_QSTR_type, MP_ARG_INT, {.u_int = ADAFRUIT_ESP32SPI_SOCK_STREAM} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    adafruit_esp32spi_socketpool_socketpool_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    adafruit_esp32spi_socketpool_socket_obj_t *sock = mp_obj_malloc_with_finaliser(adafruit_esp32spi_socketpool_socket_obj_t, &adafruit_esp32spi_socketpool_socket_type);

    common_hal_adafruit_esp32spi_socketpool_socket_construct(sock, self, args[ARG_family].u_int, args[ARG_type].u_int, 0);

    return MP_OBJ_FROM_PTR(sock);
}
static MP_DEFINE_CONST_FUN_OBJ_KW(adafruit_esp32spi_socketpool_socketpool_socket_obj, 1, adafruit_esp32spi_socketpool_socketpool_socket);

//|     def getaddrinfo(self, host: str, port: int) -> tuple:
//|         """Get address info for a hostname.
//|
//|         :param str host: Hostname to resolve
//|         :param int port: Port number
//|         :return tuple: Address info tuple
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_socketpool_socketpool_getaddrinfo(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_host, ARG_port };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_host, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_port, MP_ARG_REQUIRED | MP_ARG_INT },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    adafruit_esp32spi_socketpool_socketpool_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    const char *host = mp_obj_str_get_str(args[ARG_host].u_obj);
    uint16_t port = args[ARG_port].u_int;

    uint8_t ip[4];
    common_hal_adafruit_esp32spi_esp_spicontrol_get_host_by_name(self->esp, host, strlen(host), ip);

    mp_obj_t addr_tuple = mp_obj_new_bytes(ip, 4);
    mp_obj_t port_obj = MP_OBJ_NEW_SMALL_INT(port);
    mp_obj_t tuple_items[] = {addr_tuple, port_obj};
    mp_obj_t addr_info = mp_obj_new_tuple(2, tuple_items);

    mp_obj_t result_items[] = {
        MP_OBJ_NEW_SMALL_INT(ADAFRUIT_ESP32SPI_AF_INET),
        MP_OBJ_NEW_SMALL_INT(ADAFRUIT_ESP32SPI_SOCK_STREAM),
        MP_OBJ_NEW_SMALL_INT(0),
        mp_const_empty_bytes,
        addr_info
    };

    return mp_obj_new_tuple(5, result_items);
}
static MP_DEFINE_CONST_FUN_OBJ_KW(adafruit_esp32spi_socketpool_socketpool_getaddrinfo_obj, 3, adafruit_esp32spi_socketpool_socketpool_getaddrinfo);

static const mp_rom_map_elem_t adafruit_esp32spi_socketpool_socketpool_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_socket), MP_ROM_PTR(&adafruit_esp32spi_socketpool_socketpool_socket_obj) },
    { MP_ROM_QSTR(MP_QSTR_getaddrinfo), MP_ROM_PTR(&adafruit_esp32spi_socketpool_socketpool_getaddrinfo_obj) },

    // Constants
    { MP_ROM_QSTR(MP_QSTR_AF_INET), MP_ROM_INT(ADAFRUIT_ESP32SPI_AF_INET) },
    { MP_ROM_QSTR(MP_QSTR_SOCK_STREAM), MP_ROM_INT(ADAFRUIT_ESP32SPI_SOCK_STREAM) },
    { MP_ROM_QSTR(MP_QSTR_SOCK_DGRAM), MP_ROM_INT(ADAFRUIT_ESP32SPI_SOCK_DGRAM) },
};
static MP_DEFINE_CONST_DICT(adafruit_esp32spi_socketpool_socketpool_locals_dict, adafruit_esp32spi_socketpool_socketpool_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    adafruit_esp32spi_socketpool_socketpool_type,
    MP_QSTR_SocketPool,
    MP_TYPE_FLAG_NONE,
    make_new, adafruit_esp32spi_socketpool_socketpool_make_new,
    locals_dict, &adafruit_esp32spi_socketpool_socketpool_locals_dict
    );

// ============================================================================
// Socket class
// ============================================================================

//| class Socket:
//|     """A socket for network communication"""
//|
//|     def __enter__(self) -> Socket:
//|         """No-op used by Context Managers."""
//|         ...
//  Provided by context manager helper.

//|     def __exit__(self) -> None:
//|         """Automatically closes when exiting a context."""
//|         ...
static mp_obj_t adafruit_esp32spi_socketpool_socket_obj___exit__(size_t n_args, const mp_obj_t *args) {
    (void)n_args;
    common_hal_adafruit_esp32spi_socketpool_socket_close(args[0]);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(adafruit_esp32spi_socketpool_socket___exit___obj, 4, 4, adafruit_esp32spi_socketpool_socket_obj___exit__);

//|     def connect(self, address: tuple) -> None:
//|         """Connect to a remote address.
//|
//|         :param tuple address: (host, port) tuple
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_socketpool_socket_connect(mp_obj_t self_in, mp_obj_t address_in) {
    adafruit_esp32spi_socketpool_socket_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_obj_t *addr_items;
    size_t addr_len;
    mp_obj_get_array(address_in, &addr_len, &addr_items);

    if (addr_len != 2) {
        mp_raise_ValueError(MP_ERROR_TEXT("Address must be (host, port) tuple"));
    }

    const char *host = mp_obj_str_get_str(addr_items[0]);
    uint16_t port = mp_obj_get_int(addr_items[1]);

    common_hal_adafruit_esp32spi_socketpool_socket_connect(self, host, strlen(host), port);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(adafruit_esp32spi_socketpool_socket_connect_obj, adafruit_esp32spi_socketpool_socket_connect);

//|     def send(self, bytes: bytes) -> int:
//|         """Send data to the socket.
//|
//|         :param bytes bytes: Data to send
//|         :return int: Number of bytes sent
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_socketpool_socket_send(mp_obj_t self_in, mp_obj_t buf_in) {
    adafruit_esp32spi_socketpool_socket_obj_t *self = MP_OBJ_TO_PTR(self_in);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(buf_in, &bufinfo, MP_BUFFER_READ);

    size_t sent = common_hal_adafruit_esp32spi_socketpool_socket_send(self, bufinfo.buf, bufinfo.len);

    return MP_OBJ_NEW_SMALL_INT(sent);
}
static MP_DEFINE_CONST_FUN_OBJ_2(adafruit_esp32spi_socketpool_socket_send_obj, adafruit_esp32spi_socketpool_socket_send);

//|     def recv(self, bufsize: int) -> bytes:
//|         """Receive data from the socket.
//|
//|         :param int bufsize: Maximum bytes to receive
//|         :return bytes: Data received
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_socketpool_socket_recv(mp_obj_t self_in, mp_obj_t len_in) {
    adafruit_esp32spi_socketpool_socket_obj_t *self = MP_OBJ_TO_PTR(self_in);
    size_t len = mp_obj_get_int(len_in);

    uint8_t *buf = m_malloc(len);
    size_t received = common_hal_adafruit_esp32spi_socketpool_socket_recv_into(self, buf, len);

    mp_obj_t result = mp_obj_new_bytes(buf, received);
    m_free(buf);

    return result;
}
static MP_DEFINE_CONST_FUN_OBJ_2(adafruit_esp32spi_socketpool_socket_recv_obj, adafruit_esp32spi_socketpool_socket_recv);

//|     def recv_into(self, buffer: bytes, nbytes: int = 0) -> int:
//|         """Receive data into a buffer.
//|
//|         :param bytes buffer: Buffer to receive into
//|         :param int nbytes: Number of bytes to receive (0 = fill buffer)
//|         :return int: Number of bytes received
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_socketpool_socket_recv_into(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_buffer, ARG_nbytes };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_buffer, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_nbytes, MP_ARG_INT, {.u_int = 0} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    adafruit_esp32spi_socketpool_socket_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_buffer].u_obj, &bufinfo, MP_BUFFER_WRITE);

    size_t len = args[ARG_nbytes].u_int;
    if (len == 0 || len > bufinfo.len) {
        len = bufinfo.len;
    }

    size_t received = common_hal_adafruit_esp32spi_socketpool_socket_recv_into(self, bufinfo.buf, len);

    return MP_OBJ_NEW_SMALL_INT(received);
}
static MP_DEFINE_CONST_FUN_OBJ_KW(adafruit_esp32spi_socketpool_socket_recv_into_obj, 2, adafruit_esp32spi_socketpool_socket_recv_into);

//|     def settimeout(self, value: Optional[float]) -> None:
//|         """Set the socket timeout.
//|
//|         :param Optional[float] value: Timeout in seconds (None = blocking)
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_socketpool_socket_settimeout(mp_obj_t self_in, mp_obj_t timeout_in) {
    adafruit_esp32spi_socketpool_socket_obj_t *self = MP_OBJ_TO_PTR(self_in);

    uint32_t timeout_ms;
    if (timeout_in == mp_const_none) {
        timeout_ms = UINT32_MAX;  // Blocking
    } else {
        mp_float_t timeout_s = mp_obj_get_float(timeout_in);
        timeout_ms = (uint32_t)(timeout_s * 1000);
    }

    common_hal_adafruit_esp32spi_socketpool_socket_settimeout(self, timeout_ms);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_2(adafruit_esp32spi_socketpool_socket_settimeout_obj, adafruit_esp32spi_socketpool_socket_settimeout);

//|     def close(self) -> None:
//|         """Close the socket."""
//|         ...
static mp_obj_t adafruit_esp32spi_socketpool_socket_close(mp_obj_t self_in) {
    adafruit_esp32spi_socketpool_socket_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_adafruit_esp32spi_socketpool_socket_close(self);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(adafruit_esp32spi_socketpool_socket_close_obj, adafruit_esp32spi_socketpool_socket_close);

static const mp_rom_map_elem_t adafruit_esp32spi_socketpool_socket_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&default___enter___obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&adafruit_esp32spi_socketpool_socket___exit___obj) },
    { MP_ROM_QSTR(MP_QSTR_connect), MP_ROM_PTR(&adafruit_esp32spi_socketpool_socket_connect_obj) },
    { MP_ROM_QSTR(MP_QSTR_send), MP_ROM_PTR(&adafruit_esp32spi_socketpool_socket_send_obj) },
    { MP_ROM_QSTR(MP_QSTR_recv), MP_ROM_PTR(&adafruit_esp32spi_socketpool_socket_recv_obj) },
    { MP_ROM_QSTR(MP_QSTR_recv_into), MP_ROM_PTR(&adafruit_esp32spi_socketpool_socket_recv_into_obj) },
    { MP_ROM_QSTR(MP_QSTR_settimeout), MP_ROM_PTR(&adafruit_esp32spi_socketpool_socket_settimeout_obj) },
    { MP_ROM_QSTR(MP_QSTR_close), MP_ROM_PTR(&adafruit_esp32spi_socketpool_socket_close_obj) },
};
static MP_DEFINE_CONST_DICT(adafruit_esp32spi_socketpool_socket_locals_dict, adafruit_esp32spi_socketpool_socket_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    adafruit_esp32spi_socketpool_socket_type,
    MP_QSTR_Socket,
    MP_TYPE_FLAG_NONE,
    locals_dict, &adafruit_esp32spi_socketpool_socket_locals_dict
    );
