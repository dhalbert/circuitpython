// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Dan Halbert for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"
#include "common-hal/busio/SPI.h"
#include "common-hal/digitalio/DigitalInOut.h"

// Connection modes
typedef enum {
    ADAFRUIT_ESP32SPI_TCP_MODE = 0,
    ADAFRUIT_ESP32SPI_UDP_MODE = 1,
    ADAFRUIT_ESP32SPI_TLS_MODE = 2,
} adafruit_esp32spi_conn_mode_t;

typedef struct {
    mp_obj_base_t base;
    busio_spi_obj_t *spi;
    digitalio_digitalinout_obj_t *cs;
    digitalio_digitalinout_obj_t *ready;
    digitalio_digitalinout_obj_t *reset;
    digitalio_digitalinout_obj_t *gpio0;
    bool debug;
    bool debug_show_secrets;
    uint8_t *sendbuf;
    size_t sendbuf_len;
    uint8_t buffer[10];
    uint8_t pbuf[1];
    int8_t tls_socket;  // -1 if no TLS socket allocated
} adafruit_esp32spi_esp_spicontrol_obj_t;
