// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Dan Halbert for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include "shared-bindings/adafruit_esp32spi/ESP_SPIcontrol.h"
#include "shared-bindings/busio/SPI.h"
#include "shared-bindings/digitalio/DigitalInOut.h"
#include "shared-bindings/util.h"
#include "shared/runtime/buffer_helper.h"
#include "shared/runtime/context_manager_helpers.h"
#include "py/mperrno.h"
#include "py/runtime.h"
#include "py/objproperty.h"

//| class ESP_SPIcontrol:
//|     """ESP32 SPI WiFi Control"""
//|
//|     TCP_MODE: int
//|     """TCP connection mode"""
//|     UDP_MODE: int
//|     """UDP connection mode"""
//|     TLS_MODE: int
//|     """TLS/SSL connection mode"""
//|
//|     def __init__(
//|         self,
//|         spi: busio.SPI,
//|         cs: digitalio.DigitalInOut,
//|         ready: digitalio.DigitalInOut,
//|         reset: digitalio.DigitalInOut,
//|         gpio0: Optional[digitalio.DigitalInOut] = None,
//|         *,
//|         debug: bool = False,
//|         debug_show_secrets: bool = False
//|     ) -> None:
//|         """Create an ESP32 SPI WiFi control object.
//|
//|         :param busio.SPI spi: The SPI bus to use
//|         :param digitalio.DigitalInOut cs: Chip select pin
//|         :param digitalio.DigitalInOut ready: Ready pin
//|         :param digitalio.DigitalInOut reset: Reset pin
//|         :param digitalio.DigitalInOut gpio0: Optional GPIO0 pin for boot mode control
//|         :param bool debug: Enable debug output
//|         :param bool debug_show_secrets: Show passwords and keys in debug output
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_spi, ARG_cs, ARG_ready, ARG_reset, ARG_gpio0, ARG_debug, ARG_debug_show_secrets };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_spi, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_cs, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_ready, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_reset, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_gpio0, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_debug, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
        { MP_QSTR_debug_show_secrets, MP_ARG_KW_ONLY | MP_ARG_BOOL, {.u_bool = false} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    busio_spi_obj_t *spi = mp_arg_validate_type(args[ARG_spi].u_obj, &busio_spi_type, MP_QSTR_spi);

    digitalio_digitalinout_obj_t *cs =
        mp_arg_validate_type(args[ARG_cs].u_obj, &digitalio_digitalinout_type, MP_QSTR_cs);

    digitalio_digitalinout_obj_t *ready =
        mp_arg_validate_type(args[ARG_ready].u_obj, &digitalio_digitalinout_type, MP_QSTR_ready);

    digitalio_digitalinout_obj_t *reset =
        mp_arg_validate_type(args[ARG_reset].u_obj, &digitalio_digitalinout_type, MP_QSTR_reset);

    digitalio_digitalinout_obj_t *gpio0 =
        mp_arg_validate_type_or_none(args[ARG_gpio0].u_obj, &digitalio_digitalinout_type, MP_QSTR_gpio0);

    adafruit_esp32spi_esp_spicontrol_obj_t *self = mp_obj_malloc_with_finaliser(adafruit_esp32spi_esp_spicontrol_obj_t, &adafruit_esp32spi_esp_spicontrol_type);

    common_hal_adafruit_esp32spi_esp_spicontrol_construct(
        self, spi, cs, ready, reset, gpio0,
        args[ARG_debug].u_bool,
        args[ARG_debug_show_secrets].u_bool);

    return MP_OBJ_FROM_PTR(self);
}

//|     def deinit(self) -> None:
//|         """Deinitialize the ESP32 SPI control object."""
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_obj_deinit(mp_obj_t self_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_adafruit_esp32spi_esp_spicontrol_deinit(self);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(adafruit_esp32spi_esp_spicontrol_deinit_obj, adafruit_esp32spi_esp_spicontrol_obj_deinit);

//|     def __enter__(self) -> ESP_SPIcontrol:
//|         """No-op used by Context Managers."""
//|         ...
//  Provided by context manager helper.

//|     def __exit__(self) -> None:
//|         """Automatically deinitializes when exiting a context. See
//|         :ref:`lifetime-and-contextmanagers` for more info."""
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_obj___exit__(size_t n_args, const mp_obj_t *args) {
    (void)n_args;
    common_hal_adafruit_esp32spi_esp_spicontrol_deinit(args[0]);
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(adafruit_esp32spi_esp_spicontrol___exit___obj, 4, 4, adafruit_esp32spi_esp_spicontrol_obj___exit__);

//|     def reset(self) -> None:
//|         """Reset the co-processor using its reset pin."""
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_reset(mp_obj_t self_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_adafruit_esp32spi_esp_spicontrol_reset(self);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(adafruit_esp32spi_esp_spicontrol_reset_obj, adafruit_esp32spi_esp_spicontrol_reset);

//|     status: int
//|     """The WiFi connection status. Can be `WL_NO_SHIELD`, `WL_NO_MODULE`, `WL_IDLE_STATUS`,
//|     `WL_NO_SSID_AVAIL`, `WL_SCAN_COMPLETED`, `WL_CONNECTED`, `WL_CONNECT_FAILED`,
//|     `WL_CONNECTION_LOST`, `WL_DISCONNECTED`, `WL_AP_LISTENING`, `WL_AP_CONNECTED`, `WL_AP_FAILED`."""
static mp_obj_t adafruit_esp32spi_esp_spicontrol_get_status(mp_obj_t self_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_adafruit_esp32spi_esp_spicontrol_get_status(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(adafruit_esp32spi_esp_spicontrol_get_status_obj, adafruit_esp32spi_esp_spicontrol_get_status);

MP_PROPERTY_GETTER(adafruit_esp32spi_esp_spicontrol_status_obj,
    (mp_obj_t)&adafruit_esp32spi_esp_spicontrol_get_status_obj);

//|     firmware_version: str
//|     """The firmware version running on the co-processor."""
static mp_obj_t adafruit_esp32spi_esp_spicontrol_get_firmware_version(mp_obj_t self_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    char buf[32];
    common_hal_adafruit_esp32spi_esp_spicontrol_get_firmware_version(self, buf, sizeof(buf));
    return mp_obj_new_str(buf, strlen(buf));
}
MP_DEFINE_CONST_FUN_OBJ_1(adafruit_esp32spi_esp_spicontrol_get_firmware_version_obj, adafruit_esp32spi_esp_spicontrol_get_firmware_version);

MP_PROPERTY_GETTER(adafruit_esp32spi_esp_spicontrol_firmware_version_obj,
    (mp_obj_t)&adafruit_esp32spi_esp_spicontrol_get_firmware_version_obj);

//|     mac_address: bytes
//|     """The MAC address as a bytes object."""
static mp_obj_t adafruit_esp32spi_esp_spicontrol_get_mac_address(mp_obj_t self_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t mac[6];
    common_hal_adafruit_esp32spi_esp_spicontrol_get_mac_address(self, mac);
    return mp_obj_new_bytes(mac, 6);
}
MP_DEFINE_CONST_FUN_OBJ_1(adafruit_esp32spi_esp_spicontrol_get_mac_address_obj, adafruit_esp32spi_esp_spicontrol_get_mac_address);

MP_PROPERTY_GETTER(adafruit_esp32spi_esp_spicontrol_mac_address_obj,
    (mp_obj_t)&adafruit_esp32spi_esp_spicontrol_get_mac_address_obj);

//|     connected: bool
//|     """``True`` if connected to an access point."""
static mp_obj_t adafruit_esp32spi_esp_spicontrol_get_connected(mp_obj_t self_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return mp_obj_new_bool(common_hal_adafruit_esp32spi_esp_spicontrol_get_connected(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(adafruit_esp32spi_esp_spicontrol_get_connected_obj, adafruit_esp32spi_esp_spicontrol_get_connected);

MP_PROPERTY_GETTER(adafruit_esp32spi_esp_spicontrol_connected_obj,
    (mp_obj_t)&adafruit_esp32spi_esp_spicontrol_get_connected_obj);

//|     ip_address: bytes
//|     """The current IP address as a bytes object."""
static mp_obj_t adafruit_esp32spi_esp_spicontrol_get_ip_address(mp_obj_t self_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t ip[4];
    common_hal_adafruit_esp32spi_esp_spicontrol_get_ip_address(self, ip);
    return mp_obj_new_bytes(ip, 4);
}
MP_DEFINE_CONST_FUN_OBJ_1(adafruit_esp32spi_esp_spicontrol_get_ip_address_obj, adafruit_esp32spi_esp_spicontrol_get_ip_address);

MP_PROPERTY_GETTER(adafruit_esp32spi_esp_spicontrol_ip_address_obj,
    (mp_obj_t)&adafruit_esp32spi_esp_spicontrol_get_ip_address_obj);

//|     def connect_AP(
//|         self, ssid: Union[str, bytes], password: Union[str, bytes], timeout_s: float = 10
//|     ) -> None:
//|         """Connect to a WiFi access point.
//|
//|         :param Union[str, bytes] ssid: The SSID of the network
//|         :param Union[str, bytes] password: The password for the network
//|         :param float timeout_s: Connection timeout in seconds
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_connect_AP(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_ssid, ARG_password, ARG_timeout_s };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_ssid, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_password, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_timeout_s, MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    mp_buffer_info_t ssid_info;
    mp_get_buffer_raise(args[ARG_ssid].u_obj, &ssid_info, MP_BUFFER_READ);

    mp_buffer_info_t password_info;
    mp_get_buffer_raise(args[ARG_password].u_obj, &password_info, MP_BUFFER_READ);

    mp_float_t timeout = 10.0;
    if (args[ARG_timeout_s].u_obj != MP_OBJ_NULL) {
        timeout = mp_obj_get_float(args[ARG_timeout_s].u_obj);
    }

    common_hal_adafruit_esp32spi_esp_spicontrol_connect_AP(
        self,
        ssid_info.buf, ssid_info.len,
        password_info.buf, password_info.len,
        timeout);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(adafruit_esp32spi_esp_spicontrol_connect_AP_obj, 3, adafruit_esp32spi_esp_spicontrol_connect_AP);

//|     def disconnect(self) -> None:
//|         """Disconnect from the access point."""
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_disconnect(mp_obj_t self_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_adafruit_esp32spi_esp_spicontrol_disconnect(self);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(adafruit_esp32spi_esp_spicontrol_disconnect_obj, adafruit_esp32spi_esp_spicontrol_disconnect);

//|     def get_host_by_name(self, hostname: str) -> bytes:
//|         """Get IP address for a hostname.
//|
//|         :param str hostname: The hostname to resolve
//|         :return bytes: The IP address as 4 bytes
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_get_host_by_name(mp_obj_t self_in, mp_obj_t hostname_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    const char *hostname = mp_obj_str_get_str(hostname_in);
    uint8_t ip[4];
    common_hal_adafruit_esp32spi_esp_spicontrol_get_host_by_name(self, hostname, strlen(hostname), ip);
    return mp_obj_new_bytes(ip, 4);
}
MP_DEFINE_CONST_FUN_OBJ_2(adafruit_esp32spi_esp_spicontrol_get_host_by_name_obj, adafruit_esp32spi_esp_spicontrol_get_host_by_name);

//|     def ping(self, dest: Union[str, bytes], ttl: int = 250) -> int:
//|         """Ping a destination.
//|
//|         :param Union[str, bytes] dest: Hostname or IP address
//|         :param int ttl: Time to live
//|         :return int: Ping time in milliseconds
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_ping(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_dest, ARG_ttl };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_dest, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_ttl, MP_ARG_INT, {.u_int = 250} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    uint8_t dest[4];
    if (mp_obj_is_str(args[ARG_dest].u_obj)) {
        const char *hostname = mp_obj_str_get_str(args[ARG_dest].u_obj);
        common_hal_adafruit_esp32spi_esp_spicontrol_get_host_by_name(self, hostname, strlen(hostname), dest);
    } else {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[ARG_dest].u_obj, &bufinfo, MP_BUFFER_READ);
        if (bufinfo.len != 4) {
            mp_raise_ValueError_varg(MP_ERROR_TEXT("Address must be %d bytes long"), 4);
        }
        memcpy(dest, bufinfo.buf, 4);
    }

    uint8_t ttl = (uint8_t)mp_arg_validate_int_range(args[ARG_ttl].u_int, 0, 255, MP_QSTR_ttl);

    uint16_t ms = common_hal_adafruit_esp32spi_esp_spicontrol_ping(self, dest, ttl);
    return MP_OBJ_NEW_SMALL_INT(ms);
}
MP_DEFINE_CONST_FUN_OBJ_KW(adafruit_esp32spi_esp_spicontrol_ping_obj, 2, adafruit_esp32spi_esp_spicontrol_ping);

//|     def get_socket(self) -> int:
//|         """Get an available socket number.
//|
//|         :return int: Socket number
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_get_socket(mp_obj_t self_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t sock = common_hal_adafruit_esp32spi_esp_spicontrol_get_socket(self);
    if (sock == 255) {
        mp_raise_OSError(MP_ENFILE);
    }
    return MP_OBJ_NEW_SMALL_INT(sock);
}
MP_DEFINE_CONST_FUN_OBJ_1(adafruit_esp32spi_esp_spicontrol_get_socket_obj, adafruit_esp32spi_esp_spicontrol_get_socket);

//|     def socket_connect(self, socket_num: int, dest: Union[str, bytes], port: int, conn_mode: int = TCP_MODE) -> None:
//|         """Connect a socket to a destination.
//|
//|         :param int socket_num: Socket number
//|         :param Union[str, bytes] dest: Destination hostname or IP address
//|         :param int port: Port number
//|         :param int conn_mode: Connection mode (TCP_MODE, UDP_MODE, or TLS_MODE)
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_socket_connect(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_socket_num, ARG_dest, ARG_port, ARG_conn_mode };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_socket_num, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_dest, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_port, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_conn_mode, MP_ARG_INT, {.u_int = ADAFRUIT_ESP32SPI_TCP_MODE} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    uint8_t socket_num = mp_arg_validate_int_range(args[ARG_socket_num].u_int, 0, 255, MP_QSTR_socket_num);
    uint16_t port = mp_arg_validate_int_range(args[ARG_port].u_int, 0, 65535, MP_QSTR_port);
    adafruit_esp32spi_conn_mode_t conn_mode = (adafruit_esp32spi_conn_mode_t)args[ARG_conn_mode].u_int;

    mp_buffer_info_t dest_info;
    mp_get_buffer_raise(args[ARG_dest].u_obj, &dest_info, MP_BUFFER_READ);

    common_hal_adafruit_esp32spi_esp_spicontrol_socket_connect(self, socket_num, dest_info.buf, dest_info.len, port, conn_mode);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(adafruit_esp32spi_esp_spicontrol_socket_connect_obj, 4, adafruit_esp32spi_esp_spicontrol_socket_connect);

//|     def socket_status(self, socket_num: int) -> int:
//|         """Get the status of a socket.
//|
//|         :param int socket_num: Socket number
//|         :return int: Socket status
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_socket_status(mp_obj_t self_in, mp_obj_t socket_num_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t socket_num = mp_arg_validate_int_range(mp_obj_get_int(socket_num_in), 0, 255, MP_QSTR_socket_num);
    return MP_OBJ_NEW_SMALL_INT(common_hal_adafruit_esp32spi_esp_spicontrol_socket_status(self, socket_num));
}
MP_DEFINE_CONST_FUN_OBJ_2(adafruit_esp32spi_esp_spicontrol_socket_status_obj, adafruit_esp32spi_esp_spicontrol_socket_status);

//|     def socket_write(self, socket_num: int, buffer: bytes, conn_mode: int = TCP_MODE) -> None:
//|         """Write data to a socket.
//|
//|         :param int socket_num: Socket number
//|         :param bytes buffer: Data to write
//|         :param int conn_mode: Connection mode
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_socket_write(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_socket_num, ARG_buffer, ARG_conn_mode };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_socket_num, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_buffer, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_conn_mode, MP_ARG_INT, {.u_int = ADAFRUIT_ESP32SPI_TCP_MODE} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    uint8_t socket_num = mp_arg_validate_int_range(args[ARG_socket_num].u_int, 0, 255, MP_QSTR_socket_num);
    adafruit_esp32spi_conn_mode_t conn_mode = (adafruit_esp32spi_conn_mode_t)args[ARG_conn_mode].u_int;

    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[ARG_buffer].u_obj, &bufinfo, MP_BUFFER_READ);

    common_hal_adafruit_esp32spi_esp_spicontrol_socket_write(self, socket_num, bufinfo.buf, bufinfo.len, conn_mode);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(adafruit_esp32spi_esp_spicontrol_socket_write_obj, 3, adafruit_esp32spi_esp_spicontrol_socket_write);

//|     def socket_available(self, socket_num: int) -> int:
//|         """Get the number of bytes available to read from a socket.
//|
//|         :param int socket_num: Socket number
//|         :return int: Number of bytes available
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_socket_available(mp_obj_t self_in, mp_obj_t socket_num_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t socket_num = mp_arg_validate_int_range(mp_obj_get_int(socket_num_in), 0, 255, MP_QSTR_socket_num);
    return MP_OBJ_NEW_SMALL_INT(common_hal_adafruit_esp32spi_esp_spicontrol_socket_available(self, socket_num));
}
MP_DEFINE_CONST_FUN_OBJ_2(adafruit_esp32spi_esp_spicontrol_socket_available_obj, adafruit_esp32spi_esp_spicontrol_socket_available);

//|     def socket_read(self, socket_num: int, size: int) -> bytes:
//|         """Read data from a socket.
//|
//|         :param int socket_num: Socket number
//|         :param int size: Maximum number of bytes to read
//|         :return bytes: Data read
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_socket_read(mp_obj_t self_in, mp_obj_t socket_num_in, mp_obj_t size_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t socket_num = mp_arg_validate_int_range(mp_obj_get_int(socket_num_in), 0, 255, MP_QSTR_socket_num);
    size_t size = mp_obj_get_int(size_in);

    uint8_t *buffer = m_malloc(size);
    size_t bytes_read = common_hal_adafruit_esp32spi_esp_spicontrol_socket_read(self, socket_num, buffer, size);

    mp_obj_t result = mp_obj_new_bytes(buffer, bytes_read);
    m_free(buffer);

    return result;
}
MP_DEFINE_CONST_FUN_OBJ_3(adafruit_esp32spi_esp_spicontrol_socket_read_obj, adafruit_esp32spi_esp_spicontrol_socket_read);

//|     def socket_close(self, socket_num: int) -> None:
//|         """Close a socket.
//|
//|         :param int socket_num: Socket number
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_socket_close(mp_obj_t self_in, mp_obj_t socket_num_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t socket_num = mp_arg_validate_int_range(mp_obj_get_int(socket_num_in), 0, 255, MP_QSTR_socket_num);
    common_hal_adafruit_esp32spi_esp_spicontrol_socket_close(self, socket_num);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(adafruit_esp32spi_esp_spicontrol_socket_close_obj, adafruit_esp32spi_esp_spicontrol_socket_close);

//|     def start_scan_networks(self) -> None:
//|         """Start scanning for access points."""
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_start_scan_networks(mp_obj_t self_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    common_hal_adafruit_esp32spi_esp_spicontrol_start_scan_networks(self);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_1(adafruit_esp32spi_esp_spicontrol_start_scan_networks_obj, adafruit_esp32spi_esp_spicontrol_start_scan_networks);

//|     def start_server(self, port: int, socket_num: int, conn_mode: int = TCP_MODE) -> None:
//|         """Start a server on a port.
//|
//|         :param int port: Port number
//|         :param int socket_num: Socket number to use
//|         :param int conn_mode: Connection mode
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_start_server(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_port, ARG_socket_num, ARG_conn_mode };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_port, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_socket_num, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_conn_mode, MP_ARG_INT, {.u_int = ADAFRUIT_ESP32SPI_TCP_MODE} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    uint16_t port = mp_arg_validate_int_range(args[ARG_port].u_int, 0, 65535, MP_QSTR_port);
    uint8_t socket_num = mp_arg_validate_int_range(args[ARG_socket_num].u_int, 0, 255, MP_QSTR_socket_num);
    adafruit_esp32spi_conn_mode_t conn_mode = (adafruit_esp32spi_conn_mode_t)args[ARG_conn_mode].u_int;

    common_hal_adafruit_esp32spi_esp_spicontrol_start_server(self, port, socket_num, conn_mode);

    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_KW(adafruit_esp32spi_esp_spicontrol_start_server_obj, 3, adafruit_esp32spi_esp_spicontrol_start_server);

//|     def server_state(self, socket_num: int) -> int:
//|         """Get the state of a server socket.
//|
//|         :param int socket_num: Socket number
//|         :return int: Server state
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_server_state(mp_obj_t self_in, mp_obj_t socket_num_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t socket_num = mp_arg_validate_int_range(mp_obj_get_int(socket_num_in), 0, 255, MP_QSTR_socket_num);
    return MP_OBJ_NEW_SMALL_INT(common_hal_adafruit_esp32spi_esp_spicontrol_server_state(self, socket_num));
}
MP_DEFINE_CONST_FUN_OBJ_2(adafruit_esp32spi_esp_spicontrol_server_state_obj, adafruit_esp32spi_esp_spicontrol_server_state);

//|     def set_pin_mode(self, pin: int, mode: int) -> None:
//|         """Set the mode of a GPIO pin on the ESP32.
//|
//|         :param int pin: Pin number
//|         :param int mode: Pin mode (0=input, 1=output)
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_set_pin_mode(mp_obj_t self_in, mp_obj_t pin_in, mp_obj_t mode_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t pin = mp_obj_get_int(pin_in);
    uint8_t mode = mp_obj_get_int(mode_in);
    common_hal_adafruit_esp32spi_esp_spicontrol_set_pin_mode(self, pin, mode);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(adafruit_esp32spi_esp_spicontrol_set_pin_mode_obj, adafruit_esp32spi_esp_spicontrol_set_pin_mode);

//|     def set_digital_write(self, pin: int, value: bool) -> None:
//|         """Set the digital output value of a GPIO pin on the ESP32.
//|
//|         :param int pin: Pin number
//|         :param bool value: Pin value
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_set_digital_write(mp_obj_t self_in, mp_obj_t pin_in, mp_obj_t value_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t pin = mp_obj_get_int(pin_in);
    bool value = mp_obj_is_true(value_in);
    common_hal_adafruit_esp32spi_esp_spicontrol_set_digital_write(self, pin, value);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(adafruit_esp32spi_esp_spicontrol_set_digital_write_obj, adafruit_esp32spi_esp_spicontrol_set_digital_write);

//|     def set_analog_write(self, pin: int, value: float) -> None:
//|         """Set the PWM output value of a GPIO pin on the ESP32.
//|
//|         :param int pin: Pin number
//|         :param float value: PWM value (0.0 to 1.0)
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_set_analog_write(mp_obj_t self_in, mp_obj_t pin_in, mp_obj_t value_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t pin = mp_obj_get_int(pin_in);
    mp_float_t analog_value = mp_obj_get_float(value_in);
    uint8_t value = (uint8_t)(analog_value * 255);
    common_hal_adafruit_esp32spi_esp_spicontrol_set_analog_write(self, pin, value);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_3(adafruit_esp32spi_esp_spicontrol_set_analog_write_obj, adafruit_esp32spi_esp_spicontrol_set_analog_write);

//|     def set_digital_read(self, pin: int) -> bool:
//|         """Read the digital input value of a GPIO pin on the ESP32.
//|
//|         :param int pin: Pin number
//|         :return bool: Pin value
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_set_digital_read(mp_obj_t self_in, mp_obj_t pin_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t pin = mp_obj_get_int(pin_in);
    return mp_obj_new_bool(common_hal_adafruit_esp32spi_esp_spicontrol_set_digital_read(self, pin));
}
MP_DEFINE_CONST_FUN_OBJ_2(adafruit_esp32spi_esp_spicontrol_set_digital_read_obj, adafruit_esp32spi_esp_spicontrol_set_digital_read);

//|     def set_analog_read(self, pin: int, atten: int = 3) -> int:
//|         """Read the analog input value of a GPIO pin on the ESP32.
//|
//|         :param int pin: Pin number
//|         :param int atten: ADC attenuation (0-3)
//|         :return int: Analog value (0-65536)
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_set_analog_read(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) {
    enum { ARG_pin, ARG_atten };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_pin, MP_ARG_REQUIRED | MP_ARG_INT },
        { MP_QSTR_atten, MP_ARG_INT, {.u_int = 3} },  // ADC_ATTEN_DB_11
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);

    uint8_t pin = mp_arg_validate_int_range(args[ARG_pin].u_int, 0, 255, MP_QSTR_pin);
    uint8_t atten = mp_arg_validate_int_range(args[ARG_atten].u_int, 0, 3, MP_QSTR_atten);

    return MP_OBJ_NEW_SMALL_INT(common_hal_adafruit_esp32spi_esp_spicontrol_set_analog_read(self, pin, atten));
}
static MP_DEFINE_CONST_FUN_OBJ_KW(adafruit_esp32spi_esp_spicontrol_set_analog_read_obj, 2, adafruit_esp32spi_esp_spicontrol_set_analog_read);

//|     def get_time(self) -> int:
//|         """Get the current time from the ESP32.
//|
//|         :return int: Unix timestamp
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_get_time(mp_obj_t self_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_adafruit_esp32spi_esp_spicontrol_get_time(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(adafruit_esp32spi_esp_spicontrol_get_time_obj, adafruit_esp32spi_esp_spicontrol_get_time);

//|     def set_esp_debug(self, enabled: bool) -> None:
//|         """Enable or disable debug output from the ESP32.
//|
//|         :param bool enabled: Debug enable state
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_esp_spicontrol_set_esp_debug(mp_obj_t self_in, mp_obj_t enabled_in) {
    adafruit_esp32spi_esp_spicontrol_obj_t *self = MP_OBJ_TO_PTR(self_in);
    bool enabled = mp_obj_is_true(enabled_in);
    common_hal_adafruit_esp32spi_esp_spicontrol_set_esp_debug(self, enabled);
    return mp_const_none;
}
MP_DEFINE_CONST_FUN_OBJ_2(adafruit_esp32spi_esp_spicontrol_set_esp_debug_obj, adafruit_esp32spi_esp_spicontrol_set_esp_debug);

static const mp_rom_map_elem_t adafruit_esp32spi_esp_spicontrol_locals_dict_table[] = {
    // Methods
    { MP_ROM_QSTR(MP_QSTR_deinit), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_deinit_obj) },
    { MP_ROM_QSTR(MP_QSTR___enter__), MP_ROM_PTR(&default___enter___obj) },
    { MP_ROM_QSTR(MP_QSTR___exit__), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol___exit___obj) },
    { MP_ROM_QSTR(MP_QSTR_reset), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_reset_obj) },
    { MP_ROM_QSTR(MP_QSTR_connect_AP), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_connect_AP_obj) },
    { MP_ROM_QSTR(MP_QSTR_disconnect), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_disconnect_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_host_by_name), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_get_host_by_name_obj) },
    { MP_ROM_QSTR(MP_QSTR_ping), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_ping_obj) },
    { MP_ROM_QSTR(MP_QSTR_get_socket), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_get_socket_obj) },

    // Socket methods
    { MP_ROM_QSTR(MP_QSTR_socket_connect), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_socket_connect_obj) },
    { MP_ROM_QSTR(MP_QSTR_socket_status), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_socket_status_obj) },
    { MP_ROM_QSTR(MP_QSTR_socket_write), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_socket_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_socket_available), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_socket_available_obj) },
    { MP_ROM_QSTR(MP_QSTR_socket_read), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_socket_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_socket_close), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_socket_close_obj) },

    // Network scanning
    { MP_ROM_QSTR(MP_QSTR_start_scan_networks), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_start_scan_networks_obj) },

    // Server methods
    { MP_ROM_QSTR(MP_QSTR_start_server), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_start_server_obj) },
    { MP_ROM_QSTR(MP_QSTR_server_state), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_server_state_obj) },

    // GPIO methods
    { MP_ROM_QSTR(MP_QSTR_set_pin_mode), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_set_pin_mode_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_digital_write), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_set_digital_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_analog_write), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_set_analog_write_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_digital_read), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_set_digital_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_analog_read), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_set_analog_read_obj) },

    // Utility methods
    { MP_ROM_QSTR(MP_QSTR_get_time), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_get_time_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_esp_debug), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_set_esp_debug_obj) },

    // Properties
    { MP_ROM_QSTR(MP_QSTR_status), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_status_obj) },
    { MP_ROM_QSTR(MP_QSTR_firmware_version), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_firmware_version_obj) },
    { MP_ROM_QSTR(MP_QSTR_mac_address), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_mac_address_obj) },
    { MP_ROM_QSTR(MP_QSTR_connected), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_connected_obj) },
    { MP_ROM_QSTR(MP_QSTR_ip_address), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_ip_address_obj) },

    // Connection mode constants
    { MP_ROM_QSTR(MP_QSTR_TCP_MODE), MP_ROM_INT(ADAFRUIT_ESP32SPI_TCP_MODE) },
    { MP_ROM_QSTR(MP_QSTR_UDP_MODE), MP_ROM_INT(ADAFRUIT_ESP32SPI_UDP_MODE) },
    { MP_ROM_QSTR(MP_QSTR_TLS_MODE), MP_ROM_INT(ADAFRUIT_ESP32SPI_TLS_MODE) },
};
static MP_DEFINE_CONST_DICT(adafruit_esp32spi_esp_spicontrol_locals_dict, adafruit_esp32spi_esp_spicontrol_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    adafruit_esp32spi_esp_spicontrol_type,
    MP_QSTR_ESP_SPIcontrol,
    MP_TYPE_FLAG_HAS_SPECIAL_ACCESSORS,
    make_new, adafruit_esp32spi_esp_spicontrol_make_new,
    locals_dict, &adafruit_esp32spi_esp_spicontrol_locals_dict
    );
