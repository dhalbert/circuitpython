// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Dan Halbert for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"
#include "shared-module/adafruit_esp32spi/ESP_SPIcontrol.h"

typedef struct {
    mp_obj_base_t base;
    // Reference to ESP_SPIcontrol object for querying if needed
    adafruit_esp32spi_esp_spicontrol_obj_t *esp_spi_control;
    // Cached data from scan or NULL if should query ESP32
    uint8_t *raw_ssid;
    size_t raw_ssid_len;
    uint8_t *raw_bssid;
    size_t raw_bssid_len;
    int32_t raw_rssi;
    uint8_t raw_channel;
    uint8_t *raw_country;
    size_t raw_country_len;
    uint8_t raw_authmode;
    // Flags to indicate which fields have cached data
    bool has_raw_ssid;
    bool has_raw_bssid;
    bool has_raw_rssi;
    bool has_raw_channel;
    bool has_raw_country;
    bool has_raw_authmode;
} adafruit_esp32spi_network_obj_t;
