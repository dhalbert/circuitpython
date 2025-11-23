// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Dan Halbert for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#pragma once

typedef enum {
    WL_NO_SHIELD = 255,
    WL_NO_MODULE = 255,
    WL_STOPPED = 254,
    WL_IDLE_STATUS = 0,
    WL_NO_SSID_AVAIL = 1,
    WL_SCAN_COMPLETED = 2,
    WL_CONNECTED = 3,
    WL_CONNECT_FAILED = 4,
    WL_CONNECTION_LOST = 5,
    WL_DISCONNECTED = 6,
    WL_AP_LISTENING = 7,
    WL_AP_CONNECTED = 8,
    WL_AP_FAILED = 9,
} adafruit_esp32spi_wl_status_t;

typedef enum {
    WL_TCP_CLOSED = 0,
    WL_TCP_LISTEN = 1,
    WL_TCP_SYN_SENT = 2,
    WL_TCP_SYN_RCVD = 3,
    WL_TCP_ESTABLISHED = 4,
    WL_TCP_FIN_WAIT_1 = 5,
    WL_TCP_FIN_WAIT_2 = 6,
    WL_TCP_CLOSE_WAIT = 7,
    WL_TCP_CLOSING = 8,
    WL_TCP_LAST_ACK = 9,
    WL_TCP_TIME_WAIT = 10,
} adafruit_esp32spi_wl_tcp_state_t;
