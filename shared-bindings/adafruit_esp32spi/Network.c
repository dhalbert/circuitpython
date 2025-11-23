// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Dan Halbert for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include "shared-bindings/adafruit_esp32spi/Network.h"
#include "shared-bindings/adafruit_esp32spi/ESP_SPIcontrol.h"
#include "py/runtime.h"
#include "py/objproperty.h"

#include <string.h>

//| class Network:
//|     """A WiFi network provided by a nearby access point."""
//|
//|     def __init__(
//|         self,
//|         esp_spi_control: ESP_SPIcontrol,
//|         raw_ssid: Optional[bytes] = None,
//|         raw_bssid: Optional[bytes] = None,
//|         raw_rssi: Optional[int] = None,
//|         raw_channel: Optional[int] = None,
//|         raw_country: Optional[bytes] = None,
//|         raw_authmode: Optional[int] = None,
//|     ) -> None:
//|         """Create a Network object. If raw_* parameters are not provided, the Network will
//|         query the ESP32 for current connection information when properties are accessed.
//|
//|         :param ESP_SPIcontrol esp_spi_control: The ESP_SPIcontrol object
//|         :param bytes raw_ssid: Optional cached SSID
//|         :param bytes raw_bssid: Optional cached BSSID
//|         :param int raw_rssi: Optional cached RSSI
//|         :param int raw_channel: Optional cached channel number
//|         :param bytes raw_country: Optional cached country code
//|         :param int raw_authmode: Optional cached authentication mode
//|         """
//|         ...
static mp_obj_t adafruit_esp32spi_network_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    enum { ARG_esp_spi_control, ARG_raw_ssid, ARG_raw_bssid, ARG_raw_rssi, ARG_raw_channel, ARG_raw_country, ARG_raw_authmode };
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_esp_spi_control, MP_ARG_REQUIRED | MP_ARG_OBJ },
        { MP_QSTR_raw_ssid, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_raw_bssid, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_raw_rssi, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_raw_channel, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_raw_country, MP_ARG_OBJ, {.u_obj = mp_const_none} },
        { MP_QSTR_raw_authmode, MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    adafruit_esp32spi_esp_spicontrol_obj_t *esp_spi_control =
        mp_arg_validate_type(args[ARG_esp_spi_control].u_obj, &adafruit_esp32spi_esp_spicontrol_type, MP_QSTR_esp_spi_control);

    adafruit_esp32spi_network_obj_t *self = mp_obj_malloc(adafruit_esp32spi_network_obj_t, &adafruit_esp32spi_network_type);

    // Parse optional cached data
    const uint8_t *raw_ssid = NULL;
    size_t raw_ssid_len = 0;
    if (args[ARG_raw_ssid].u_obj != mp_const_none) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[ARG_raw_ssid].u_obj, &bufinfo, MP_BUFFER_READ);
        raw_ssid = bufinfo.buf;
        raw_ssid_len = bufinfo.len;
    }

    const uint8_t *raw_bssid = NULL;
    size_t raw_bssid_len = 0;
    if (args[ARG_raw_bssid].u_obj != mp_const_none) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[ARG_raw_bssid].u_obj, &bufinfo, MP_BUFFER_READ);
        raw_bssid = bufinfo.buf;
        raw_bssid_len = bufinfo.len;
    }

    int32_t raw_rssi = 0;
    bool has_raw_rssi = false;
    if (args[ARG_raw_rssi].u_obj != mp_const_none) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[ARG_raw_rssi].u_obj, &bufinfo, MP_BUFFER_READ);
        if (bufinfo.len >= 4) {
            // Unpack as little-endian int32
            raw_rssi = ((int32_t *)bufinfo.buf)[0];
            has_raw_rssi = true;
        }
    }

    uint8_t raw_channel = 0;
    bool has_raw_channel = false;
    if (args[ARG_raw_channel].u_obj != mp_const_none) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[ARG_raw_channel].u_obj, &bufinfo, MP_BUFFER_READ);
        if (bufinfo.len > 0) {
            raw_channel = ((uint8_t *)bufinfo.buf)[0];
            has_raw_channel = true;
        }
    }

    const uint8_t *raw_country = NULL;
    size_t raw_country_len = 0;
    if (args[ARG_raw_country].u_obj != mp_const_none) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[ARG_raw_country].u_obj, &bufinfo, MP_BUFFER_READ);
        raw_country = bufinfo.buf;
        raw_country_len = bufinfo.len;
    }

    uint8_t raw_authmode = 0;
    bool has_raw_authmode = false;
    if (args[ARG_raw_authmode].u_obj != mp_const_none) {
        mp_buffer_info_t bufinfo;
        mp_get_buffer_raise(args[ARG_raw_authmode].u_obj, &bufinfo, MP_BUFFER_READ);
        if (bufinfo.len > 0) {
            raw_authmode = ((uint8_t *)bufinfo.buf)[0];
            has_raw_authmode = true;
        }
    }

    common_hal_adafruit_esp32spi_network_construct(
        self, esp_spi_control,
        raw_ssid, raw_ssid_len,
        raw_bssid, raw_bssid_len,
        raw_rssi, has_raw_rssi,
        raw_channel, has_raw_channel,
        raw_country, raw_country_len,
        raw_authmode, has_raw_authmode);

    return MP_OBJ_FROM_PTR(self);
}

//|     ssid: str
//|     """The SSID of the network as a string."""
static mp_obj_t adafruit_esp32spi_network_get_ssid(mp_obj_t self_in) {
    adafruit_esp32spi_network_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t ssid_buf[33];  // Max SSID length is 32 + null terminator
    size_t ssid_len = 0;
    common_hal_adafruit_esp32spi_network_get_ssid(self, ssid_buf, &ssid_len);
    return mp_obj_new_str((const char *)ssid_buf, ssid_len);
}
MP_DEFINE_CONST_FUN_OBJ_1(adafruit_esp32spi_network_get_ssid_obj, adafruit_esp32spi_network_get_ssid);

MP_PROPERTY_GETTER(adafruit_esp32spi_network_ssid_obj,
    (mp_obj_t)&adafruit_esp32spi_network_get_ssid_obj);

//|     bssid: bytes
//|     """The BSSID of the network as bytes (usually the AP's MAC address)."""
static mp_obj_t adafruit_esp32spi_network_get_bssid(mp_obj_t self_in) {
    adafruit_esp32spi_network_obj_t *self = MP_OBJ_TO_PTR(self_in);
    uint8_t bssid[6];
    common_hal_adafruit_esp32spi_network_get_bssid(self, bssid);
    return mp_obj_new_bytes(bssid, 6);
}
MP_DEFINE_CONST_FUN_OBJ_1(adafruit_esp32spi_network_get_bssid_obj, adafruit_esp32spi_network_get_bssid);

MP_PROPERTY_GETTER(adafruit_esp32spi_network_bssid_obj,
    (mp_obj_t)&adafruit_esp32spi_network_get_bssid_obj);

//|     rssi: int
//|     """The signal strength (RSSI) of the network in dBm."""
static mp_obj_t adafruit_esp32spi_network_get_rssi(mp_obj_t self_in) {
    adafruit_esp32spi_network_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return MP_OBJ_NEW_SMALL_INT(common_hal_adafruit_esp32spi_network_get_rssi(self));
}
MP_DEFINE_CONST_FUN_OBJ_1(adafruit_esp32spi_network_get_rssi_obj, adafruit_esp32spi_network_get_rssi);

MP_PROPERTY_GETTER(adafruit_esp32spi_network_rssi_obj,
    (mp_obj_t)&adafruit_esp32spi_network_get_rssi_obj);

//|     channel: Optional[int]
//|     """The channel number the network is operating on, or None if not available."""
static mp_obj_t adafruit_esp32spi_network_get_channel(mp_obj_t self_in) {
    adafruit_esp32spi_network_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return common_hal_adafruit_esp32spi_network_get_channel(self);
}
MP_DEFINE_CONST_FUN_OBJ_1(adafruit_esp32spi_network_get_channel_obj, adafruit_esp32spi_network_get_channel);

MP_PROPERTY_GETTER(adafruit_esp32spi_network_channel_obj,
    (mp_obj_t)&adafruit_esp32spi_network_get_channel_obj);

//|     country: Optional[str]
//|     """The country code, or None if not available."""
static mp_obj_t adafruit_esp32spi_network_get_country(mp_obj_t self_in) {
    adafruit_esp32spi_network_obj_t *self = MP_OBJ_TO_PTR(self_in);
    return common_hal_adafruit_esp32spi_network_get_country(self);
}
MP_DEFINE_CONST_FUN_OBJ_1(adafruit_esp32spi_network_get_country_obj, adafruit_esp32spi_network_get_country);

MP_PROPERTY_GETTER(adafruit_esp32spi_network_country_obj,
    (mp_obj_t)&adafruit_esp32spi_network_get_country_obj);

//|     authmode: str
//|     """The authentication mode as a string: "OPEN", "WEP", "PSK", "WPA2", or "UNKNOWN"."""
static mp_obj_t adafruit_esp32spi_network_get_authmode(mp_obj_t self_in) {
    adafruit_esp32spi_network_obj_t *self = MP_OBJ_TO_PTR(self_in);
    char authmode_buf[10];
    common_hal_adafruit_esp32spi_network_get_authmode(self, authmode_buf, sizeof(authmode_buf));
    return mp_obj_new_str(authmode_buf, strlen(authmode_buf));
}
MP_DEFINE_CONST_FUN_OBJ_1(adafruit_esp32spi_network_get_authmode_obj, adafruit_esp32spi_network_get_authmode);

MP_PROPERTY_GETTER(adafruit_esp32spi_network_authmode_obj,
    (mp_obj_t)&adafruit_esp32spi_network_get_authmode_obj);

static const mp_rom_map_elem_t adafruit_esp32spi_network_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_ssid), MP_ROM_PTR(&adafruit_esp32spi_network_ssid_obj) },
    { MP_ROM_QSTR(MP_QSTR_bssid), MP_ROM_PTR(&adafruit_esp32spi_network_bssid_obj) },
    { MP_ROM_QSTR(MP_QSTR_rssi), MP_ROM_PTR(&adafruit_esp32spi_network_rssi_obj) },
    { MP_ROM_QSTR(MP_QSTR_channel), MP_ROM_PTR(&adafruit_esp32spi_network_channel_obj) },
    { MP_ROM_QSTR(MP_QSTR_country), MP_ROM_PTR(&adafruit_esp32spi_network_country_obj) },
    { MP_ROM_QSTR(MP_QSTR_authmode), MP_ROM_PTR(&adafruit_esp32spi_network_authmode_obj) },
};
static MP_DEFINE_CONST_DICT(adafruit_esp32spi_network_locals_dict, adafruit_esp32spi_network_locals_dict_table);

MP_DEFINE_CONST_OBJ_TYPE(
    adafruit_esp32spi_network_type,
    MP_QSTR_Network,
    MP_TYPE_FLAG_NONE,
    make_new, adafruit_esp32spi_network_make_new,
    locals_dict, &adafruit_esp32spi_network_locals_dict
    );
