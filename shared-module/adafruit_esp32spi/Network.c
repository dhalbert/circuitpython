// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Dan Halbert for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include "shared-bindings/adafruit_esp32spi/Network.h"
#include "shared-bindings/adafruit_esp32spi/ESP_SPIcontrol.h"
#include "py/runtime.h"

#include <string.h>

void common_hal_adafruit_esp32spi_network_construct(
    adafruit_esp32spi_network_obj_t *self,
    adafruit_esp32spi_esp_spicontrol_obj_t *esp_spi_control,
    const uint8_t *raw_ssid, size_t raw_ssid_len,
    const uint8_t *raw_bssid, size_t raw_bssid_len,
    int32_t raw_rssi, bool has_raw_rssi,
    uint8_t raw_channel, bool has_raw_channel,
    const uint8_t *raw_country, size_t raw_country_len,
    uint8_t raw_authmode, bool has_raw_authmode) {

    self->esp_spi_control = esp_spi_control;

    // Copy cached SSID if provided
    if (raw_ssid != NULL && raw_ssid_len > 0) {
        self->raw_ssid = m_malloc(raw_ssid_len);
        memcpy(self->raw_ssid, raw_ssid, raw_ssid_len);
        self->raw_ssid_len = raw_ssid_len;
        self->has_raw_ssid = true;
    } else {
        self->raw_ssid = NULL;
        self->raw_ssid_len = 0;
        self->has_raw_ssid = false;
    }

    // Copy cached BSSID if provided
    if (raw_bssid != NULL && raw_bssid_len > 0) {
        self->raw_bssid = m_malloc(raw_bssid_len);
        memcpy(self->raw_bssid, raw_bssid, raw_bssid_len);
        self->raw_bssid_len = raw_bssid_len;
        self->has_raw_bssid = true;
    } else {
        self->raw_bssid = NULL;
        self->raw_bssid_len = 0;
        self->has_raw_bssid = false;
    }

    // Store cached RSSI if provided
    self->raw_rssi = raw_rssi;
    self->has_raw_rssi = has_raw_rssi;

    // Store cached channel if provided
    self->raw_channel = raw_channel;
    self->has_raw_channel = has_raw_channel;

    // Copy cached country if provided
    if (raw_country != NULL && raw_country_len > 0) {
        self->raw_country = m_malloc(raw_country_len);
        memcpy(self->raw_country, raw_country, raw_country_len);
        self->raw_country_len = raw_country_len;
        self->has_raw_country = true;
    } else {
        self->raw_country = NULL;
        self->raw_country_len = 0;
        self->has_raw_country = false;
    }

    // Store cached authmode if provided
    self->raw_authmode = raw_authmode;
    self->has_raw_authmode = has_raw_authmode;
}

void common_hal_adafruit_esp32spi_network_get_ssid(
    adafruit_esp32spi_network_obj_t *self,
    uint8_t *ssid_buf,
    size_t *ssid_len) {

    if (self->has_raw_ssid) {
        *ssid_len = self->raw_ssid_len;
        memcpy(ssid_buf, self->raw_ssid, self->raw_ssid_len);
    } else {
        // Query ESP32 for current SSID
        common_hal_adafruit_esp32spi_esp_spicontrol_get_curr_ssid(
            self->esp_spi_control, ssid_buf, ssid_len);
    }
}

void common_hal_adafruit_esp32spi_network_get_bssid(
    adafruit_esp32spi_network_obj_t *self,
    uint8_t *bssid) {

    if (self->has_raw_bssid) {
        memcpy(bssid, self->raw_bssid, 6);
    } else {
        // Query ESP32 for current BSSID
        common_hal_adafruit_esp32spi_esp_spicontrol_get_curr_bssid(
            self->esp_spi_control, bssid);
    }
}

int32_t common_hal_adafruit_esp32spi_network_get_rssi(
    adafruit_esp32spi_network_obj_t *self) {

    if (self->has_raw_rssi) {
        return self->raw_rssi;
    } else {
        // Query ESP32 for current RSSI
        return common_hal_adafruit_esp32spi_esp_spicontrol_get_curr_rssi(
            self->esp_spi_control);
    }
}

mp_obj_t common_hal_adafruit_esp32spi_network_get_channel(
    adafruit_esp32spi_network_obj_t *self) {

    if (self->has_raw_channel) {
        return MP_OBJ_NEW_SMALL_INT(self->raw_channel);
    } else {
        // Channel is only available from scan results, not from current connection
        return mp_const_none;
    }
}

mp_obj_t common_hal_adafruit_esp32spi_network_get_country(
    adafruit_esp32spi_network_obj_t *self) {

    if (self->has_raw_country) {
        return mp_obj_new_str((const char *)self->raw_country, self->raw_country_len);
    } else {
        // Country is only available from scan results
        return mp_const_none;
    }
}

void common_hal_adafruit_esp32spi_network_get_authmode(
    adafruit_esp32spi_network_obj_t *self,
    char *authmode_buf,
    size_t buf_len) {

    uint8_t authmode;

    if (self->has_raw_authmode) {
        authmode = self->raw_authmode;
    } else {
        // Query ESP32 for current encryption type
        authmode = common_hal_adafruit_esp32spi_esp_spicontrol_get_curr_enct(
            self->esp_spi_control);
    }

    // Convert authmode to string based on Nina firmware mapping
    // https://github.com/adafruit/nina-fw/blob/master/arduino/libraries/WiFi/src/WiFi.cpp#L385
    const char *authmode_str;
    if (authmode == 7) {
        authmode_str = "OPEN";
    } else if (authmode == 5) {
        authmode_str = "WEP";
    } else if (authmode == 2) {
        authmode_str = "PSK";
    } else if (authmode == 4) {
        authmode_str = "WPA2";
    } else {
        authmode_str = "UNKNOWN";
    }

    strncpy(authmode_buf, authmode_str, buf_len - 1);
    authmode_buf[buf_len - 1] = '\0';
}
