// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Dan Halbert for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"
#include "shared-module/adafruit_esp32spi/Network.h"
#include "shared-module/adafruit_esp32spi/ESP_SPIcontrol.h"

extern const mp_obj_type_t adafruit_esp32spi_network_type;

extern void common_hal_adafruit_esp32spi_network_construct(
    adafruit_esp32spi_network_obj_t *self,
    adafruit_esp32spi_esp_spicontrol_obj_t *esp_spi_control,
    const uint8_t *raw_ssid, size_t raw_ssid_len,
    const uint8_t *raw_bssid, size_t raw_bssid_len,
    int32_t raw_rssi, bool has_raw_rssi,
    uint8_t raw_channel, bool has_raw_channel,
    const uint8_t *raw_country, size_t raw_country_len,
    uint8_t raw_authmode, bool has_raw_authmode);

extern void common_hal_adafruit_esp32spi_network_get_ssid(
    adafruit_esp32spi_network_obj_t *self, uint8_t *ssid_buf, size_t *ssid_len);

extern void common_hal_adafruit_esp32spi_network_get_bssid(
    adafruit_esp32spi_network_obj_t *self, uint8_t *bssid);

extern int32_t common_hal_adafruit_esp32spi_network_get_rssi(
    adafruit_esp32spi_network_obj_t *self);

extern mp_obj_t common_hal_adafruit_esp32spi_network_get_channel(
    adafruit_esp32spi_network_obj_t *self);

extern mp_obj_t common_hal_adafruit_esp32spi_network_get_country(
    adafruit_esp32spi_network_obj_t *self);

extern void common_hal_adafruit_esp32spi_network_get_authmode(
    adafruit_esp32spi_network_obj_t *self, char *authmode_buf, size_t buf_len);
