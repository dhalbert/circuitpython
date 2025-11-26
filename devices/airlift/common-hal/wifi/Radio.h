// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Dan Halbert for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#pragma once

#include "py/obj.h"

#include "shared-bindings/busio/SPI.h"
#include "shared-bindings/digitalio/DigitalInOut.h"
#include "shared-bindings/wifi/ScannedNetworks.h"
#include "shared-bindings/wifi/Network.h"

// Connection modes
typedef enum {
    ADAFRUIT_ESP32SPI_TCP_MODE = 0,
    ADAFRUIT_ESP32SPI_UDP_MODE = 1,
    ADAFRUIT_ESP32SPI_TLS_MODE = 2,
} adafruit_esp32spi_conn_mode_t;

typedef struct {
    mp_obj_base_t base;
} adafruit_esp32spi_esp_spicontrol_obj_t;
typedef struct {
    mp_obj_base_t base;
    wifi_scannednetworks_obj_t *current_scan;
    uint32_t ping_elapsed_time;
    bool started;
    bool ap_mode;
    bool sta_mode;
    uint8_t retries_left;
    uint8_t starting_retries;
    uint8_t last_disconnect_reason;

    busio_spi_obj_t *spi;
    digitalio_digitalinout_obj_t *cs;
    digitalio_digitalinout_obj_t *ready;
    digitalio_digitalinout_obj_t *reset;
    digitalio_digitalinout_obj_t *gpio0;
    uint8_t *sendbuf;
    size_t sendbuf_len;
    uint8_t buffer[10];
    uint8_t pbuf[1];
    int8_t tls_socket;  // -1 if no TLS socket allocated
} wifi_radio_obj_t;

extern void common_hal_wifi_radio_gc_collect(wifi_radio_obj_t *self);

// ESP32 SPI protocol command constants
#define _START_CMD 0xE0
#define _END_CMD 0xEE
#define _ERR_CMD 0xEF
#define _REPLY_FLAG 0x80
#define _CMD_FLAG 0

// Command opcodes
#define _SET_NET_CMD 0x10
#define _SET_PASSPHRASE_CMD 0x11
#define _SET_IP_CONFIG 0x14
#define _SET_DNS_CONFIG 0x15
#define _SET_HOSTNAME 0x16
#define _SET_AP_NET_CMD 0x18
#define _SET_AP_PASSPHRASE_CMD 0x19
#define _SET_DEBUG_CMD 0x1A

#define _GET_CONN_STATUS_CMD 0x20
#define _GET_IPADDR_CMD 0x21
#define _GET_MACADDR_CMD 0x22
#define _GET_CURR_SSID_CMD 0x23
#define _GET_CURR_BSSID_CMD 0x24
#define _GET_CURR_RSSI_CMD 0x25
#define _GET_CURR_ENCT_CMD 0x26

#define _SCAN_NETWORKS 0x27
#define _START_SERVER_TCP_CMD 0x28
#define _GET_STATE_TCP_CMD 0x29
#define _DATA_SENT_TCP_CMD 0x2A
#define _AVAIL_DATA_TCP_CMD 0x2B
#define _GET_DATA_TCP_CMD 0x2C
#define _START_CLIENT_TCP_CMD 0x2D
#define _STOP_CLIENT_TCP_CMD 0x2E
#define _GET_CLIENT_STATE_TCP_CMD 0x2F
#define _DISCONNECT_CMD 0x30
#define _GET_IDX_RSSI_CMD 0x32
#define _GET_IDX_ENCT_CMD 0x33
#define _REQ_HOST_BY_NAME_CMD 0x34
#define _GET_HOST_BY_NAME_CMD 0x35
#define _START_SCAN_NETWORKS 0x36
#define _GET_FW_VERSION_CMD 0x37
#define _SEND_UDP_DATA_CMD 0x39
#define _GET_REMOTE_DATA_CMD 0x3A
#define _GET_TIME 0x3B
#define _GET_IDX_BSSID_CMD 0x3C
#define _GET_IDX_CHAN_CMD 0x3D
#define _PING_CMD 0x3E
#define _GET_SOCKET_CMD 0x3F

#define _SET_CLI_CERT 0x40
#define _SET_PK 0x41
#define _SEND_DATA_TCP_CMD 0x44
#define _GET_DATABUF_TCP_CMD 0x45
#define _INSERT_DATABUF_TCP_CMD 0x46
#define _SET_ENT_IDENT_CMD 0x4A
#define _SET_ENT_UNAME_CMD 0x4B
#define _SET_ENT_PASSWD_CMD 0x4C
#define _SET_ENT_ENABLE_CMD 0x4F

#define _SET_PIN_MODE_CMD 0x50
#define _SET_DIGITAL_WRITE_CMD 0x51
#define _SET_ANALOG_WRITE_CMD 0x52
#define _SET_DIGITAL_READ_CMD 0x53
#define _SET_ANALOG_READ_CMD 0x54

#define DEFAULT_SENDBUF_SIZE 256
#define SOCKET_CHUNK_SIZE 64

// Communication with AirLift.
void wifi_radio_send_command(wifi_radio_obj_t *self, uint8_t cmd,
    const uint8_t **params, const size_t *param_lens, size_t num_params);
uint8_t wifi_radio_read_byte(wifi_radio_obj_t *self);
void wifi_radio_wait_spi_char(wifi_radio_obj_t *self, uint8_t desired);
void wifi_radio_check_data(wifi_radio_obj_t *self, uint8_t desired);
size_t wifi_radio_wait_response_cmd(wifi_radio_obj_t *self, uint8_t cmd,
    uint8_t **responses, size_t *response_lens, size_t max_responses);
size_t wifi_radio_send_command_get_response(wifi_radio_obj_t *self, uint8_t cmd,
    const uint8_t **params, const size_t *param_lens, size_t num_params,
    uint8_t **responses, size_t *response_lens, size_t max_responses);
