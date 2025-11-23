// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Dan Halbert for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"
#include "common-hal/busio/SPI.h"
#include "common-hal/digitalio/DigitalInOut.h"
#include "shared-bindings/adafruit_esp32spi/__init__.h"
#include "shared-module/adafruit_esp32spi/ESP_SPIcontrol.h"

extern const mp_obj_type_t adafruit_esp32spi_esp_spicontrol_type;

extern void common_hal_adafruit_esp32spi_esp_spicontrol_construct(adafruit_esp32spi_esp_spicontrol_obj_t *self, busio_spi_obj_t *spi, digitalio_digitalinout_obj_t *cs, digitalio_digitalinout_obj_t *ready, digitalio_digitalinout_obj_t *reset, digitalio_digitalinout_obj_t *gpio0, bool debug, bool debug_show_secrets);

extern void common_hal_adafruit_esp32spi_esp_spicontrol_mark_deinit(adafruit_esp32spi_esp_spicontrol_obj_t *self);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_deinit(adafruit_esp32spi_esp_spicontrol_obj_t *self);
extern bool common_hal_adafruit_esp32spi_esp_spicontrol_deinited(adafruit_esp32spi_esp_spicontrol_obj_t *self);

extern void common_hal_adafruit_esp32spi_esp_spicontrol_reset(adafruit_esp32spi_esp_spicontrol_obj_t *self);
extern adafruit_esp32spi_wl_status_t common_hal_adafruit_esp32spi_esp_spicontrol_get_status(adafruit_esp32spi_esp_spicontrol_obj_t *self);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_get_firmware_version(adafruit_esp32spi_esp_spicontrol_obj_t *self, char *buf, size_t buf_len);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_get_mac_address(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t *mac);
extern bool common_hal_adafruit_esp32spi_esp_spicontrol_get_connected(adafruit_esp32spi_esp_spicontrol_obj_t *self);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_connect_AP(adafruit_esp32spi_esp_spicontrol_obj_t *self,
    const uint8_t *ssid, size_t ssid_len, const uint8_t *password, size_t password_len, mp_float_t timeout_s);

extern void common_hal_adafruit_esp32spi_esp_spicontrol_disconnect(adafruit_esp32spi_esp_spicontrol_obj_t *self);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_get_ip_address(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t *ip);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_get_host_by_name(adafruit_esp32spi_esp_spicontrol_obj_t *self, const char *hostname, size_t hostname_len, uint8_t *ip);
extern uint16_t common_hal_adafruit_esp32spi_esp_spicontrol_ping(adafruit_esp32spi_esp_spicontrol_obj_t *self, const uint8_t *dest, uint8_t ttl);

extern uint8_t common_hal_adafruit_esp32spi_esp_spicontrol_get_socket(adafruit_esp32spi_esp_spicontrol_obj_t *self);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_socket_connect(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t socket_num, const uint8_t *dest, size_t dest_len, uint16_t port, adafruit_esp32spi_conn_mode_t conn_mode);
extern adafruit_esp32spi_wl_tcp_state_t common_hal_adafruit_esp32spi_esp_spicontrol_socket_status(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t socket_num);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_socket_write(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t socket_num, const uint8_t *buffer, size_t len, adafruit_esp32spi_conn_mode_t conn_mode);
extern uint16_t common_hal_adafruit_esp32spi_esp_spicontrol_socket_available(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t socket_num);
extern size_t common_hal_adafruit_esp32spi_esp_spicontrol_socket_read(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t socket_num, uint8_t *buffer, size_t size);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_socket_close(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t socket_num);

// Network scanning
extern void common_hal_adafruit_esp32spi_esp_spicontrol_start_scan_networks(adafruit_esp32spi_esp_spicontrol_obj_t *self);

// Current network information (for Network class)
extern void common_hal_adafruit_esp32spi_esp_spicontrol_get_curr_ssid(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t *ssid, size_t *ssid_len);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_get_curr_bssid(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t *bssid);
extern int32_t common_hal_adafruit_esp32spi_esp_spicontrol_get_curr_rssi(adafruit_esp32spi_esp_spicontrol_obj_t *self);
extern uint8_t common_hal_adafruit_esp32spi_esp_spicontrol_get_curr_enct(adafruit_esp32spi_esp_spicontrol_obj_t *self);

// Server operations
extern void common_hal_adafruit_esp32spi_esp_spicontrol_start_server(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint16_t port, uint8_t socket_num, adafruit_esp32spi_conn_mode_t conn_mode);
extern uint8_t common_hal_adafruit_esp32spi_esp_spicontrol_server_state(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t socket_num);

// GPIO operations
extern void common_hal_adafruit_esp32spi_esp_spicontrol_set_pin_mode(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t pin, uint8_t mode);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_set_digital_write(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t pin, bool value);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_set_analog_write(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t pin, uint8_t value);
extern bool common_hal_adafruit_esp32spi_esp_spicontrol_set_digital_read(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t pin);
extern uint16_t common_hal_adafruit_esp32spi_esp_spicontrol_set_analog_read(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t pin, uint8_t atten);

// Time operations
extern uint32_t common_hal_adafruit_esp32spi_esp_spicontrol_get_time(adafruit_esp32spi_esp_spicontrol_obj_t *self);

// Debug operations
extern void common_hal_adafruit_esp32spi_esp_spicontrol_set_esp_debug(adafruit_esp32spi_esp_spicontrol_obj_t *self, bool enabled);

// WiFi configuration operations
extern void common_hal_adafruit_esp32spi_esp_spicontrol_wifi_set_network(adafruit_esp32spi_esp_spicontrol_obj_t *self, const uint8_t *ssid, size_t ssid_len);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_wifi_set_passphrase(adafruit_esp32spi_esp_spicontrol_obj_t *self, const uint8_t *ssid, size_t ssid_len, const uint8_t *passphrase, size_t passphrase_len);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_set_ip_config(adafruit_esp32spi_esp_spicontrol_obj_t *self, const uint8_t *ip, const uint8_t *gateway, const uint8_t *mask);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_set_dns_config(adafruit_esp32spi_esp_spicontrol_obj_t *self, const uint8_t *dns1, const uint8_t *dns2);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_set_hostname(adafruit_esp32spi_esp_spicontrol_obj_t *self, const char *hostname, size_t hostname_len);

// Enterprise WiFi operations
extern void common_hal_adafruit_esp32spi_esp_spicontrol_wifi_set_entidentity(adafruit_esp32spi_esp_spicontrol_obj_t *self, const uint8_t *identity, size_t identity_len);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_wifi_set_entusername(adafruit_esp32spi_esp_spicontrol_obj_t *self, const uint8_t *username, size_t username_len);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_wifi_set_entpassword(adafruit_esp32spi_esp_spicontrol_obj_t *self, const uint8_t *password, size_t password_len);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_wifi_set_entenable(adafruit_esp32spi_esp_spicontrol_obj_t *self);

// Certificate operations
extern void common_hal_adafruit_esp32spi_esp_spicontrol_set_certificate(adafruit_esp32spi_esp_spicontrol_obj_t *self, const uint8_t *certificate, size_t certificate_len);
extern void common_hal_adafruit_esp32spi_esp_spicontrol_set_private_key(adafruit_esp32spi_esp_spicontrol_obj_t *self, const uint8_t *private_key, size_t private_key_len);

// Remote connection info
extern void common_hal_adafruit_esp32spi_esp_spicontrol_get_remote_data(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t socket_num, uint8_t *ip, uint16_t *port);
