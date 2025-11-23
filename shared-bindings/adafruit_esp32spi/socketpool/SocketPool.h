// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Dan Halbert for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"
#include "shared-module/adafruit_esp32spi/socketpool/SocketPool.h"

extern const mp_obj_type_t adafruit_esp32spi_socketpool_socketpool_type;
extern const mp_obj_type_t adafruit_esp32spi_socketpool_socket_type;

// SocketPool methods
extern void common_hal_adafruit_esp32spi_socketpool_socketpool_construct(
    adafruit_esp32spi_socketpool_socketpool_obj_t *self,
    adafruit_esp32spi_esp_spicontrol_obj_t *esp);

extern void common_hal_adafruit_esp32spi_socketpool_socketpool_deinit(adafruit_esp32spi_socketpool_socketpool_obj_t *self);
extern bool common_hal_adafruit_esp32spi_socketpool_socketpool_deinited(adafruit_esp32spi_socketpool_socketpool_obj_t *self);

// Socket methods
extern void common_hal_adafruit_esp32spi_socketpool_socket_construct(
    adafruit_esp32spi_socketpool_socket_obj_t *self,
    adafruit_esp32spi_socketpool_socketpool_obj_t *pool,
    int family,
    int type,
    int proto);

extern void common_hal_adafruit_esp32spi_socketpool_socket_connect(
    adafruit_esp32spi_socketpool_socket_obj_t *self,
    const char *host,
    size_t host_len,
    uint16_t port);

extern void common_hal_adafruit_esp32spi_socketpool_socket_close(adafruit_esp32spi_socketpool_socket_obj_t *self);
extern bool common_hal_adafruit_esp32spi_socketpool_socket_closed(adafruit_esp32spi_socketpool_socket_obj_t *self);

extern size_t common_hal_adafruit_esp32spi_socketpool_socket_send(
    adafruit_esp32spi_socketpool_socket_obj_t *self,
    const uint8_t *buf,
    size_t len);

extern size_t common_hal_adafruit_esp32spi_socketpool_socket_recv_into(
    adafruit_esp32spi_socketpool_socket_obj_t *self,
    uint8_t *buf,
    size_t len);

extern void common_hal_adafruit_esp32spi_socketpool_socket_settimeout(
    adafruit_esp32spi_socketpool_socket_obj_t *self,
    uint32_t timeout_ms);

extern uint16_t common_hal_adafruit_esp32spi_socketpool_socket_available(adafruit_esp32spi_socketpool_socket_obj_t *self);
