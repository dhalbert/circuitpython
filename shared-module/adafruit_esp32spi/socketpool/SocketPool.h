// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Dan Halbert for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"
#include "shared-module/adafruit_esp32spi/ESP_SPIcontrol.h"

// Socket family and type constants
#define ADAFRUIT_ESP32SPI_AF_INET 2
#define ADAFRUIT_ESP32SPI_SOCK_STREAM 1
#define ADAFRUIT_ESP32SPI_SOCK_DGRAM 2

typedef struct {
    mp_obj_base_t base;
    adafruit_esp32spi_esp_spicontrol_obj_t *esp;
} adafruit_esp32spi_socketpool_socketpool_obj_t;

typedef struct {
    mp_obj_base_t base;
    adafruit_esp32spi_socketpool_socketpool_obj_t *pool;
    uint8_t socket_num;
    int family;
    int type;
    uint32_t timeout_ms;
    bool connected;
} adafruit_esp32spi_socketpool_socket_obj_t;
