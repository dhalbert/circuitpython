// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Dan Halbert for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include "shared-bindings/adafruit_esp32spi/socketpool/SocketPool.h"
#include "shared-bindings/adafruit_esp32spi/ESP_SPIcontrol.h"
#include "py/runtime.h"
#include "py/mperrno.h"
#include "py/mphal.h"

#include <string.h>

#define NO_SOCKET_AVAIL (255)

// SocketPool implementation

void common_hal_adafruit_esp32spi_socketpool_socketpool_construct(
    adafruit_esp32spi_socketpool_socketpool_obj_t *self,
    adafruit_esp32spi_esp_spicontrol_obj_t *esp) {

    self->esp = esp;
}

void common_hal_adafruit_esp32spi_socketpool_socketpool_deinit(adafruit_esp32spi_socketpool_socketpool_obj_t *self) {
    self->esp = NULL;
}

bool common_hal_adafruit_esp32spi_socketpool_socketpool_deinited(adafruit_esp32spi_socketpool_socketpool_obj_t *self) {
    return self->esp == NULL;
}

// Socket implementation

void common_hal_adafruit_esp32spi_socketpool_socket_construct(
    adafruit_esp32spi_socketpool_socket_obj_t *self,
    adafruit_esp32spi_socketpool_socketpool_obj_t *pool,
    int family,
    int type,
    int proto) {

    if (family != ADAFRUIT_ESP32SPI_AF_INET) {
        mp_raise_ValueError(MP_ERROR_TEXT("Only AF_INET family supported"));
    }

    self->pool = pool;
    self->family = family;
    self->type = type;
    self->timeout_ms = 0;  // No timeout by default
    self->connected = false;

    // Get a socket from ESP32
    self->socket_num = common_hal_adafruit_esp32spi_esp_spicontrol_get_socket(pool->esp);
    if (self->socket_num == NO_SOCKET_AVAIL) {
        mp_raise_OSError(MP_ENFILE);
    }
}

void common_hal_adafruit_esp32spi_socketpool_socket_connect(
    adafruit_esp32spi_socketpool_socket_obj_t *self,
    const char *host,
    size_t host_len,
    uint16_t port) {

    // Determine connection mode based on socket type
    adafruit_esp32spi_conn_mode_t conn_mode =
        (self->type == ADAFRUIT_ESP32SPI_SOCK_DGRAM) ?
        ADAFRUIT_ESP32SPI_UDP_MODE : ADAFRUIT_ESP32SPI_TCP_MODE;

    // Connect the socket
    common_hal_adafruit_esp32spi_esp_spicontrol_socket_connect(
        self->pool->esp,
        self->socket_num,
        (const uint8_t *)host,
        host_len,
        port,
        conn_mode);

    // For TCP, wait for connection to establish
    if (conn_mode == ADAFRUIT_ESP32SPI_TCP_MODE) {
        uint64_t start_ms = mp_hal_ticks_ms();
        uint32_t timeout = (self->timeout_ms > 0) ? self->timeout_ms : 3000;

        while ((mp_hal_ticks_ms() - start_ms) < timeout) {
            uint8_t status = common_hal_adafruit_esp32spi_esp_spicontrol_socket_status(
                self->pool->esp, self->socket_num);

            if (status == WL_TCP_ESTABLISHED) {
                self->connected = true;
                return;
            }

            mp_hal_delay_ms(10);
            RUN_BACKGROUND_TASKS;
        }

        mp_raise_OSError(MP_ETIMEDOUT);
    } else {
        // UDP doesn't have a connection state
        self->connected = true;
    }
}

void common_hal_adafruit_esp32spi_socketpool_socket_close(adafruit_esp32spi_socketpool_socket_obj_t *self) {
    if (self->socket_num != NO_SOCKET_AVAIL) {
        common_hal_adafruit_esp32spi_esp_spicontrol_socket_close(self->pool->esp, self->socket_num);
        self->socket_num = NO_SOCKET_AVAIL;
        self->connected = false;
    }
}

bool common_hal_adafruit_esp32spi_socketpool_socket_closed(adafruit_esp32spi_socketpool_socket_obj_t *self) {
    return self->socket_num == NO_SOCKET_AVAIL;
}

size_t common_hal_adafruit_esp32spi_socketpool_socket_send(
    adafruit_esp32spi_socketpool_socket_obj_t *self,
    const uint8_t *buf,
    size_t len) {

    if (!self->connected) {
        mp_raise_OSError(MP_ENOTCONN);
    }

    adafruit_esp32spi_conn_mode_t conn_mode =
        (self->type == ADAFRUIT_ESP32SPI_SOCK_DGRAM) ?
        ADAFRUIT_ESP32SPI_UDP_MODE : ADAFRUIT_ESP32SPI_TCP_MODE;

    common_hal_adafruit_esp32spi_esp_spicontrol_socket_write(
        self->pool->esp,
        self->socket_num,
        buf,
        len,
        conn_mode);

    return len;
}

size_t common_hal_adafruit_esp32spi_socketpool_socket_recv_into(
    adafruit_esp32spi_socketpool_socket_obj_t *self,
    uint8_t *buf,
    size_t len) {

    if (!self->connected) {
        mp_raise_OSError(MP_ENOTCONN);
    }

    uint64_t start_ms = mp_hal_ticks_ms();
    size_t total_read = 0;

    while (total_read < len) {
        uint16_t available = common_hal_adafruit_esp32spi_esp_spicontrol_socket_available(
            self->pool->esp, self->socket_num);

        if (available > 0) {
            size_t to_read = (available < (len - total_read)) ? available : (len - total_read);
            size_t read = common_hal_adafruit_esp32spi_esp_spicontrol_socket_read(
                self->pool->esp,
                self->socket_num,
                buf + total_read,
                to_read);

            total_read += read;
            start_ms = mp_hal_ticks_ms();  // Reset timeout on successful read

            if (total_read > 0 && available == read) {
                // Got some data and no more available, return what we have
                break;
            }
        } else if (total_read > 0) {
            // Got some data but no more available
            break;
        }

        // Check timeout
        if (self->timeout_ms == 0) {
            // Non-blocking mode
            break;
        } else if (self->timeout_ms > 0) {
            uint64_t elapsed = mp_hal_ticks_ms() - start_ms;
            if (elapsed >= self->timeout_ms) {
                if (total_read == 0) {
                    mp_raise_OSError(MP_ETIMEDOUT);
                }
                break;
            }
        }

        mp_hal_delay_ms(10);
        RUN_BACKGROUND_TASKS;
    }

    return total_read;
}

void common_hal_adafruit_esp32spi_socketpool_socket_settimeout(
    adafruit_esp32spi_socketpool_socket_obj_t *self,
    uint32_t timeout_ms) {

    self->timeout_ms = timeout_ms;
}

uint16_t common_hal_adafruit_esp32spi_socketpool_socket_available(adafruit_esp32spi_socketpool_socket_obj_t *self) {
    if (!self->connected) {
        return 0;
    }

    return common_hal_adafruit_esp32spi_esp_spicontrol_socket_available(
        self->pool->esp, self->socket_num);
}
