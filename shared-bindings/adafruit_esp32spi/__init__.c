// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Dan Halbert for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include <stdint.h>

#include "py/obj.h"
#include "py/runtime.h"

#include "shared-bindings/adafruit_esp32spi/__init__.h"
#include "shared-bindings/adafruit_esp32spi/ESP_SPIcontrol.h"
#include "shared-bindings/adafruit_esp32spi/Network.h"
#include "shared-bindings/adafruit_esp32spi/socketpool/SocketPool.h"

//| """ESP32 SPI WiFi Control
//|
//| The `adafruit_esp32spi` module provides a CircuitPython driver for using the ESP32
//| as a WiFi co-processor using SPI.
//|
//| This module provides the low-level ESP_SPIcontrol class implemented in native C.
//| Higher-level functionality such as SocketPool is available as
//| Python modules in the frozen library.
//|
//| All classes change hardware state and should be deinitialized when they
//| are no longer needed. To do so, either call :py:meth:`!deinit` or use a context manager.
//|
//| For example::
//|
//|     import busio
//|     import digitalio
//|     from board import *
//|     from adafruit_esp32spi import adafruit_esp32spi
//|
//|     spi = busio.SPI(SCK, MOSI, MISO)
//|     cs = digitalio.DigitalInOut(D10)
//|     ready = digitalio.DigitalInOut(D11)
//|     reset = digitalio.DigitalInOut(D12)
//|     esp = adafruit_esp32spi.ESP_SPIcontrol(spi, cs, ready, reset)
//|
//| For socket functionality::
//|
//|     from adafruit_esp32spi.socketpool import SocketPool
//|
//|     pool = SocketPool(esp)
//| """

// Socket status constants
//| SOCKET_CLOSED: int
//| """Socket closed status"""
//|
//| SOCKET_LISTEN: int
//| """Socket listen status"""
//|
//| SOCKET_SYN_SENT: int
//| """Socket SYN sent status"""
//|
//| SOCKET_SYN_RCVD: int
//| """Socket SYN received status"""
//|
//| SOCKET_ESTABLISHED: int
//| """Socket established status"""
//|
//| SOCKET_FIN_WAIT_1: int
//| """Socket FIN wait 1 status"""
//|
//| SOCKET_FIN_WAIT_2: int
//| """Socket FIN wait 2 status"""
//|
//| SOCKET_CLOSE_WAIT: int
//| """Socket close wait status"""
//|
//| SOCKET_CLOSING: int
//| """Socket closing status"""
//|
//| SOCKET_LAST_ACK: int
//| """Socket last ACK status"""
//|
//| SOCKET_TIME_WAIT: int
//| """Socket time wait status"""
//|

// WiFi status constants
//| WL_NO_SHIELD: int
//| """No shield present"""
//|
//| WL_NO_MODULE: int
//| """No module present"""
//|
//| WL_NO_MODULE: int
//| """Not running"""
//|
//| WL_IDLE_STATUS: int
//| """WiFi idle status"""
//|
//| WL_NO_SSID_AVAIL: int
//| """No SSID available"""
//|
//| WL_SCAN_COMPLETED: int
//| """Scan completed"""
//|
//| WL_CONNECTED: int
//| """Connected to WiFi"""
//|
//| WL_CONNECT_FAILED: int
//| """Connection failed"""
//|
//| WL_CONNECTION_LOST: int
//| """Connection lost"""
//|
//| WL_DISCONNECTED: int
//| """Disconnected from WiFi"""
//|
//| WL_AP_LISTENING: int
//| """Access point listening"""
//|
//| WL_AP_CONNECTED: int
//| """Access point connected"""
//|
//| WL_AP_FAILED: int
//| """Access point failed"""
//|

// ADC attenuation constants
//| ADC_ATTEN_DB_0: int
//| """ADC attenuation 0dB"""
//|
//| ADC_ATTEN_DB_2_5: int
//| """ADC attenuation 2.5dB"""
//|
//| ADC_ATTEN_DB_6: int
//| """ADC attenuation 6dB"""
//|
//| ADC_ATTEN_DB_11: int
//| """ADC attenuation 11dB"""
//|

static const mp_map_elem_t adafruit_esp32spi_socketpool_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_socketpool) },
    { MP_ROM_QSTR(MP_QSTR_SocketPool), MP_OBJ_FROM_PTR(&adafruit_esp32spi_socketpool_socketpool_type) },
    { MP_ROM_QSTR(MP_QSTR_Socket), MP_OBJ_FROM_PTR(&adafruit_esp32spi_socketpool_socket_type) },
};

static MP_DEFINE_CONST_DICT(adafruit_esp32spi_socketpool_globals, adafruit_esp32spi_socketpool_globals_table);

static const mp_obj_module_t adafruit_esp32spi_socketpool_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&adafruit_esp32spi_socketpool_globals,
};

static const mp_rom_map_elem_t adafruit_esp32spi_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_adafruit_esp32spi) },
    { MP_ROM_QSTR(MP_QSTR_Network), MP_ROM_PTR(&adafruit_esp32spi_network_type) },
    { MP_ROM_QSTR(MP_QSTR_ESP_SPIcontrol), MP_ROM_PTR(&adafruit_esp32spi_esp_spicontrol_type) },
    { MP_ROM_QSTR(MP_QSTR_socketpool), MP_OBJ_FROM_PTR(&adafruit_esp32spi_socketpool_module) },

    // Socket status constants
    { MP_ROM_QSTR(MP_QSTR_SOCKET_CLOSED), MP_ROM_INT(WL_TCP_CLOSED) },
    { MP_ROM_QSTR(MP_QSTR_SOCKET_LISTEN), MP_ROM_INT(WL_TCP_LISTEN) },
    { MP_ROM_QSTR(MP_QSTR_SOCKET_SYN_SENT), MP_ROM_INT(WL_TCP_SYN_SENT) },
    { MP_ROM_QSTR(MP_QSTR_SOCKET_SYN_RCVD), MP_ROM_INT(WL_TCP_SYN_RCVD) },
    { MP_ROM_QSTR(MP_QSTR_SOCKET_ESTABLISHED), MP_ROM_INT(WL_TCP_ESTABLISHED) },
    { MP_ROM_QSTR(MP_QSTR_SOCKET_FIN_WAIT_1), MP_ROM_INT(WL_TCP_FIN_WAIT_1) },
    { MP_ROM_QSTR(MP_QSTR_SOCKET_FIN_WAIT_2), MP_ROM_INT(WL_TCP_FIN_WAIT_2) },
    { MP_ROM_QSTR(MP_QSTR_SOCKET_CLOSE_WAIT), MP_ROM_INT(WL_TCP_CLOSE_WAIT) },
    { MP_ROM_QSTR(MP_QSTR_SOCKET_CLOSING), MP_ROM_INT(WL_TCP_CLOSING) },
    { MP_ROM_QSTR(MP_QSTR_SOCKET_LAST_ACK), MP_ROM_INT(WL_TCP_LAST_ACK) },
    { MP_ROM_QSTR(MP_QSTR_SOCKET_TIME_WAIT), MP_ROM_INT(WL_TCP_TIME_WAIT) },

    // WiFi status constants
    { MP_ROM_QSTR(MP_QSTR_WL_NO_SHIELD), MP_ROM_INT(WL_NO_SHIELD) },
    { MP_ROM_QSTR(MP_QSTR_WL_NO_MODULE), MP_ROM_INT(WL_NO_MODULE) },
    { MP_ROM_QSTR(MP_QSTR_WL_STOPPED), MP_ROM_INT(WL_STOPPED) },
    { MP_ROM_QSTR(MP_QSTR_WL_IDLE_STATUS), MP_ROM_INT(WL_IDLE_STATUS) },
    { MP_ROM_QSTR(MP_QSTR_WL_NO_SSID_AVAIL), MP_ROM_INT(WL_NO_SSID_AVAIL) },
    { MP_ROM_QSTR(MP_QSTR_WL_SCAN_COMPLETED), MP_ROM_INT(WL_SCAN_COMPLETED) },
    { MP_ROM_QSTR(MP_QSTR_WL_CONNECTED), MP_ROM_INT(WL_CONNECTED) },
    { MP_ROM_QSTR(MP_QSTR_WL_CONNECT_FAILED), MP_ROM_INT(WL_CONNECT_FAILED) },
    { MP_ROM_QSTR(MP_QSTR_WL_CONNECTION_LOST), MP_ROM_INT(WL_CONNECTION_LOST) },
    { MP_ROM_QSTR(MP_QSTR_WL_DISCONNECTED), MP_ROM_INT(WL_DISCONNECTED) },
    { MP_ROM_QSTR(MP_QSTR_WL_AP_LISTENING), MP_ROM_INT(WL_AP_LISTENING) },
    { MP_ROM_QSTR(MP_QSTR_WL_AP_CONNECTED), MP_ROM_INT(WL_AP_CONNECTED) },
    { MP_ROM_QSTR(MP_QSTR_WL_AP_FAILED), MP_ROM_INT(WL_AP_FAILED) },

    // ADC attenuation constants
    { MP_ROM_QSTR(MP_QSTR_ADC_ATTEN_DB_0), MP_ROM_INT(0) },
    { MP_ROM_QSTR(MP_QSTR_ADC_ATTEN_DB_2_5), MP_ROM_INT(1) },
    { MP_ROM_QSTR(MP_QSTR_ADC_ATTEN_DB_6), MP_ROM_INT(2) },
    { MP_ROM_QSTR(MP_QSTR_ADC_ATTEN_DB_11), MP_ROM_INT(3) },
};

static MP_DEFINE_CONST_DICT(adafruit_esp32spi_module_globals, adafruit_esp32spi_module_globals_table);

const mp_obj_module_t adafruit_esp32spi_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&adafruit_esp32spi_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_adafruit_esp32spi, adafruit_esp32spi_module);
