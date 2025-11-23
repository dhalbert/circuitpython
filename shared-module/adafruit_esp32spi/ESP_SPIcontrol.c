// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2025 Dan Halbert for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#include "shared-bindings/adafruit_esp32spi/ESP_SPIcontrol.h"
#include "shared-bindings/busio/SPI.h"
#include "shared-bindings/digitalio/DigitalInOut.h"
#include "shared-bindings/microcontroller/Pin.h"
#include "py/mperrno.h"
#include "py/runtime.h"
#include "py/mphal.h"

#include <string.h>

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

// Release CS and lock if held.
static void spi_end_transaction(adafruit_esp32spi_esp_spicontrol_obj_t *self) {
    if (common_hal_busio_spi_has_lock(self->spi)) {
        common_hal_digitalio_digitalinout_set_value(self->cs, true);
        common_hal_busio_spi_unlock(self->spi);
    }
}

// Wait for ready pin to become low or high. Raise an exception if a timeout is exceeded.
static void wait_for_ready(adafruit_esp32spi_esp_spicontrol_obj_t *self, bool value, uint32_t timeout_ticks) {
    uint64_t start = mp_hal_ticks_ms();
    while ((mp_hal_ticks_ms() - start) < timeout_ticks) {
        if (common_hal_digitalio_digitalinout_get_value(self->ready) == value) {
            return;
        }
        RUN_BACKGROUND_TASKS;
    }

    // Timeout. Give up SPI control.
    spi_end_transaction(self);
    mp_raise_msg_varg(&mp_type_TimeoutError, MP_ERROR_TEXT("timeout waiting for ready %q"),
        value ? MP_QSTR_True : MP_QSTR_False);
}

// Wait for co-processor to be ready, then grab lock and CS.
static void spi_begin_transaction(adafruit_esp32spi_esp_spicontrol_obj_t *self) {
    // The ready line is set low when the NINA firmware is ready to start an SPI transaction.
    // Once CS is set low to signal an SPI transaction has started, NINA sets the ready line high
    // to indicate it has seen the CS transition to low.
    wait_for_ready(self, false, 10000);

    while (!common_hal_busio_spi_try_lock(self->spi)) {
        RUN_BACKGROUND_TASKS;
    }
    common_hal_busio_spi_configure(self->spi, 8000000, 0, 0, 8);

    common_hal_digitalio_digitalinout_set_value(self->cs, false);
    wait_for_ready(self, true, 1000);
}

// Send command via SPI. Assumes spi_begin_transaction() has already been called.
static void send_command(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t cmd, const uint8_t **params, const size_t *param_lens, size_t num_params) {
    // Calculate packet size
    size_t packet_len = 4;  // START + CMD + NUM_PARAMS + END
    for (size_t i = 0; i < num_params; i++) {
        packet_len += 1 + param_lens[i];  // length byte + data
    }
    // Pad to 4-byte boundary
    while (packet_len % 4 != 0) {
        packet_len++;
    }

    // Ensure buffer is large enough
    if (packet_len > self->sendbuf_len) {
        self->sendbuf = m_realloc(self->sendbuf, packet_len);
        self->sendbuf_len = packet_len;
    }

    // Build packet
    self->sendbuf[0] = _START_CMD;
    self->sendbuf[1] = cmd & ~_REPLY_FLAG;
    self->sendbuf[2] = num_params;

    size_t ptr = 3;
    for (size_t i = 0; i < num_params; i++) {
        self->sendbuf[ptr++] = param_lens[i] & 0xFF;
        memcpy(&self->sendbuf[ptr], params[i], param_lens[i]);
        ptr += param_lens[i];
    }
    self->sendbuf[ptr++] = _END_CMD;

    // Pad with zeros
    while (ptr < packet_len) {
        self->sendbuf[ptr++] = 0;
    }

    spi_begin_transaction(self);

    // Wait for ready to go high (ready to receive)
    wait_for_ready(self, true, 1000);

    common_hal_busio_spi_write(self->spi, self->sendbuf, packet_len);

    spi_end_transaction(self);
}

// Helper: Read byte from SPI
static uint8_t read_byte(adafruit_esp32spi_esp_spicontrol_obj_t *self) {
    common_hal_busio_spi_read(self->spi, self->pbuf, 1, 0xFF);
    return self->pbuf[0];
}

// Helper: Wait for specific byte
static void wait_spi_char(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t desired) {
    for (int i = 0; i < 10; i++) {
        uint8_t r = read_byte(self);
        if (r == _ERR_CMD) {
            mp_raise_msg(&mp_type_BrokenPipeError, MP_ERROR_TEXT("Error response to command"));
        }
        if (r == desired) {
            return;
        }
        mp_hal_delay_ms(10);
    }
    mp_raise_msg(&mp_type_TimeoutError, MP_ERROR_TEXT("timeout waiting for byte"));
}

// Helper: Check data matches expected
static void check_data(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t desired) {
    uint8_t r = read_byte(self);
    if (r != desired) {
        mp_raise_msg_varg(&mp_type_BrokenPipeError, MP_ERROR_TEXT("Expected %02x but got %02x"), desired, r);
    }
}

// Helper: Wait for and parse response
static size_t wait_response_cmd(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t cmd, uint8_t **responses, size_t *response_lens, size_t max_responses) {
    spi_begin_transaction(self);

    wait_spi_char(self, _START_CMD);
    check_data(self, cmd | _REPLY_FLAG);
    uint8_t num_responses = read_byte(self);

    if (num_responses > max_responses) {
        num_responses = max_responses;
    }

    for (size_t i = 0; i < num_responses; i++) {
        uint8_t param_len = read_byte(self);
        response_lens[i] = param_len;
        responses[i] = m_malloc(param_len);
        common_hal_busio_spi_read(self->spi, responses[i], param_len, 0xFF);

    }

    check_data(self, _END_CMD);

    spi_end_transaction(self);

    return num_responses;
}

// Helper: Send command and get response
static size_t send_command_get_response(adafruit_esp32spi_esp_spicontrol_obj_t *self, uint8_t cmd,
    const uint8_t **params, const size_t *param_lens, size_t num_params,
    uint8_t **responses, size_t *response_lens, size_t max_responses) {

    send_command(self, cmd, params, param_lens, num_params);
    return wait_response_cmd(self, cmd, responses, response_lens, max_responses);
}


adafruit_esp32spi_wl_status_t common_hal_adafruit_esp32spi_esp_spicontrol_get_status(adafruit_esp32spi_esp_spicontrol_obj_t *self) {
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _GET_CONN_STATUS_CMD, NULL, NULL, 0, responses, response_lens, 1);

    if (num_resp > 0 && response_lens[0] > 0) {
        adafruit_esp32spi_wl_status_t status = (adafruit_esp32spi_wl_status_t)responses[0][0];
        m_free(responses[0]);
        return status;
    }

    return WL_NO_SHIELD;
}

void common_hal_adafruit_esp32spi_esp_spicontrol_construct(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    busio_spi_obj_t *spi,
    digitalio_digitalinout_obj_t *cs,
    digitalio_digitalinout_obj_t *ready,
    digitalio_digitalinout_obj_t *reset,
    digitalio_digitalinout_obj_t *gpio0,
    bool debug,
    bool debug_show_secrets) {

    self->spi = spi;
    self->cs = cs;
    self->ready = ready;
    self->reset = reset;
    self->gpio0 = (gpio0 != mp_const_none) ? gpio0 : NULL;
    self->debug = debug;
    self->debug_show_secrets = debug_show_secrets;
    self->tls_socket = -1;

    // Allocate send buffer
    self->sendbuf_len = DEFAULT_SENDBUF_SIZE;
    self->sendbuf = m_malloc(self->sendbuf_len);

    // Configure pins
    common_hal_digitalio_digitalinout_switch_to_output(cs, true, DRIVE_MODE_PUSH_PULL);
    common_hal_digitalio_digitalinout_switch_to_input(ready, PULL_NONE);
    common_hal_digitalio_digitalinout_switch_to_output(reset, true, DRIVE_MODE_PUSH_PULL);

    if (self->gpio0) {
        common_hal_digitalio_digitalinout_switch_to_input(self->gpio0, PULL_NONE);
    }

    // Perform initial reset
    common_hal_adafruit_esp32spi_esp_spicontrol_reset(self);
}

void common_hal_adafruit_esp32spi_esp_spicontrol_mark_deinit(adafruit_esp32spi_esp_spicontrol_obj_t *self) {
    self->spi = NULL;
}

void common_hal_adafruit_esp32spi_esp_spicontrol_deinit(adafruit_esp32spi_esp_spicontrol_obj_t *self) {
    if (common_hal_adafruit_esp32spi_esp_spicontrol_deinited(self)) {
        return;
    }

    common_hal_adafruit_esp32spi_esp_spicontrol_mark_deinit(self);
}

bool common_hal_adafruit_esp32spi_esp_spicontrol_deinited(adafruit_esp32spi_esp_spicontrol_obj_t *self) {
    return self->spi == NULL;
}

void common_hal_adafruit_esp32spi_esp_spicontrol_reset(adafruit_esp32spi_esp_spicontrol_obj_t *self) {
    if (self->gpio0) {
        common_hal_digitalio_digitalinout_switch_to_output(self->gpio0, true, DRIVE_MODE_PUSH_PULL);
    }

    common_hal_digitalio_digitalinout_set_value(self->cs, true);
    common_hal_digitalio_digitalinout_set_value(self->reset, false);
    mp_hal_delay_ms(10);
    common_hal_digitalio_digitalinout_set_value(self->reset, true);
    mp_hal_delay_ms(750);  // Wait for boot

    if (self->gpio0) {
        common_hal_digitalio_digitalinout_switch_to_input(self->gpio0, PULL_NONE);
    }
}


void common_hal_adafruit_esp32spi_esp_spicontrol_get_firmware_version(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    char *buf,
    size_t buf_len) {

    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _GET_FW_VERSION_CMD, NULL, NULL, 0, responses, response_lens, 1);

    if (num_resp > 0) {
        size_t copy_len = (response_lens[0] < buf_len - 1) ? response_lens[0] : buf_len - 1;
        memcpy(buf, responses[0], copy_len);
        buf[copy_len] = '\0';
        // Remove null terminators
        for (size_t i = 0; i < copy_len; i++) {
            if (buf[i] == '\0') {
                buf[i] = ' ';
            }
        }
        m_free(responses[0]);
    } else {
        buf[0] = '\0';
    }
}

bool common_hal_adafruit_esp32spi_esp_spicontrol_get_connected(adafruit_esp32spi_esp_spicontrol_obj_t *self) {
    return common_hal_adafruit_esp32spi_esp_spicontrol_get_status(self) == WL_CONNECTED;
}

void common_hal_adafruit_esp32spi_esp_spicontrol_get_mac_address(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    uint8_t *mac) {

    const uint8_t param = 0xFF;
    const uint8_t *params[1] = { &param };
    size_t param_lens[1] = { 1 };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _GET_MACADDR_CMD, params, param_lens, 1, responses, response_lens, 1);

    if (num_resp > 0 && response_lens[0] >= 6) {
        // Reverse byte order (ESP32 returns it reversed)
        for (int i = 0; i < 6; i++) {
            mac[i] = responses[0][5 - i];
        }
        m_free(responses[0]);
    } else {
        memset(mac, 0, 6);
    }
}

void common_hal_adafruit_esp32spi_esp_spicontrol_connect_AP(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    const uint8_t *ssid,
    size_t ssid_len,
    const uint8_t *password,
    size_t password_len,
    mp_float_t timeout_s) {

    const uint8_t *params[2] = { ssid, password };
    size_t param_lens[2] = { ssid_len, password_len };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _SET_PASSPHRASE_CMD, params, param_lens, 2, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Failed to set passphrase"));
        }
    }

    // Wait for connection
    uint64_t start_ms = mp_hal_ticks_ms();
    uint64_t timeout_ms = (uint64_t)(timeout_s * 1000);

    while ((mp_hal_ticks_ms() - start_ms) < timeout_ms) {
        uint8_t status = common_hal_adafruit_esp32spi_esp_spicontrol_get_status(self);
        if (status == WL_CONNECTED) {
            return;
        }
        mp_hal_delay_ms(50);
        RUN_BACKGROUND_TASKS;
    }

    mp_raise_msg(&mp_type_ConnectionError, MP_ERROR_TEXT("Connection timeout"));
}

void common_hal_adafruit_esp32spi_esp_spicontrol_disconnect(adafruit_esp32spi_esp_spicontrol_obj_t *self) {
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _DISCONNECT_CMD, NULL, NULL, 0, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Failed to disconnect"));
        }
    }
}

void common_hal_adafruit_esp32spi_esp_spicontrol_get_ip_address(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    uint8_t *ip) {

    const uint8_t param = 0xFF;
    const uint8_t *params[1] = { &param };
    size_t param_lens[1] = { 1 };
    uint8_t *responses[3];
    size_t response_lens[3];

    size_t num_resp = send_command_get_response(self, _GET_IPADDR_CMD, params, param_lens, 1, responses, response_lens, 3);

    if (num_resp > 0 && response_lens[0] >= 4) {
        memcpy(ip, responses[0], 4);
        for (size_t i = 0; i < num_resp; i++) {
            m_free(responses[i]);
        }
    } else {
        memset(ip, 0, 4);
    }
}

void common_hal_adafruit_esp32spi_esp_spicontrol_get_host_by_name(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    const char *hostname,
    size_t hostname_len,
    uint8_t *ip) {

    // Request hostname resolution
    const uint8_t *params[1] = { (const uint8_t *)hostname };
    size_t param_lens[1] = { hostname_len };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _REQ_HOST_BY_NAME_CMD, params, param_lens, 1, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_msg(&mp_type_ConnectionError, MP_ERROR_TEXT("Failed to request hostname"));
        }
    }

    // Get the resolved IP
    num_resp = send_command_get_response(self, _GET_HOST_BY_NAME_CMD, NULL, NULL, 0, responses, response_lens, 1);

    if (num_resp > 0 && response_lens[0] >= 4) {
        memcpy(ip, responses[0], 4);
        m_free(responses[0]);
    } else {
        memset(ip, 0, 4);
    }
}

uint16_t common_hal_adafruit_esp32spi_esp_spicontrol_ping(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    const uint8_t *dest,
    uint8_t ttl) {

    const uint8_t *params[2] = { dest, &ttl };
    size_t param_lens[2] = { 4, 1 };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _PING_CMD, params, param_lens, 2, responses, response_lens, 1);

    if (num_resp > 0 && response_lens[0] >= 2) {
        uint16_t ms = responses[0][0] | (responses[0][1] << 8);
        m_free(responses[0]);
        return ms;
    }

    return 0;
}

uint8_t common_hal_adafruit_esp32spi_esp_spicontrol_get_socket(adafruit_esp32spi_esp_spicontrol_obj_t *self) {
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _GET_SOCKET_CMD, NULL, NULL, 0, responses, response_lens, 1);

    if (num_resp > 0 && response_lens[0] > 0) {
        uint8_t sock = responses[0][0];
        m_free(responses[0]);
        return sock;
    }

    return 255;  // NO_SOCKET_AVAIL
}

// Socket operations
void common_hal_adafruit_esp32spi_esp_spicontrol_socket_connect(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    uint8_t socket_num,
    const uint8_t *dest,
    size_t dest_len,
    uint16_t port,
    adafruit_esp32spi_conn_mode_t conn_mode) {

    // Check TLS socket limit
    if (conn_mode == ADAFRUIT_ESP32SPI_TLS_MODE && self->tls_socket >= 0) {
        mp_raise_OSError(MP_ENFILE);  // Only one TLS connection allowed
    }

    uint8_t port_bytes[2];
    port_bytes[0] = (port >> 8) & 0xFF;
    port_bytes[1] = port & 0xFF;

    uint8_t sock_byte = socket_num;
    uint8_t mode_byte = (uint8_t)conn_mode;

    uint8_t *responses[1];
    size_t response_lens[1];
    size_t num_resp;

    // Check if dest is a hostname (string) or IP address
    bool is_hostname = true;
    for (size_t i = 0; i < dest_len; i++) {
        if (dest[i] == 0) {
            is_hostname = false;
            break;
        }
    }

    if (is_hostname && dest_len > 0) {
        // 5-parameter version: hostname, dummy IP, port, socket, mode
        uint8_t dummy_ip[4] = {0, 0, 0, 0};
        const uint8_t *params[5] = { dest, dummy_ip, port_bytes, &sock_byte, &mode_byte };
        size_t param_lens[5] = { dest_len, 4, 2, 1, 1 };
        num_resp = send_command_get_response(self, _START_CLIENT_TCP_CMD, params, param_lens, 5, responses, response_lens, 1);
    } else {
        // 4-parameter version: IP, port, socket, mode
        const uint8_t *params[4] = { dest, port_bytes, &sock_byte, &mode_byte };
        size_t param_lens[4] = { 4, 2, 1, 1 };
        num_resp = send_command_get_response(self, _START_CLIENT_TCP_CMD, params, param_lens, 4, responses, response_lens, 1);
    }

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_ConnectionError(MP_ERROR_TEXT("Could not connect to remote server"));
        }
    }

    if (conn_mode == ADAFRUIT_ESP32SPI_TLS_MODE) {
        self->tls_socket = socket_num;
    }
}

adafruit_esp32spi_wl_tcp_state_t common_hal_adafruit_esp32spi_esp_spicontrol_socket_status(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    uint8_t socket_num) {

    uint8_t sock_byte = socket_num;
    const uint8_t *params[1] = { &sock_byte };
    size_t param_lens[1] = { 1 };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _GET_CLIENT_STATE_TCP_CMD, params, param_lens, 1, responses, response_lens, 1);

    if (num_resp > 0 && response_lens[0] > 0) {
        adafruit_esp32spi_wl_tcp_state_t status = (adafruit_esp32spi_wl_tcp_state_t)responses[0][0];
        m_free(responses[0]);
        return status;
    }

    return 0;  // SOCKET_CLOSED
}

void common_hal_adafruit_esp32spi_esp_spicontrol_socket_write(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    uint8_t socket_num,
    const uint8_t *buffer,
    size_t len,
    adafruit_esp32spi_conn_mode_t conn_mode) {

    uint8_t sock_byte = socket_num;
    size_t sent = 0;
    size_t total_chunks = (len + SOCKET_CHUNK_SIZE - 1) / SOCKET_CHUNK_SIZE;

    uint8_t send_command = (conn_mode == ADAFRUIT_ESP32SPI_UDP_MODE) ? _INSERT_DATABUF_TCP_CMD : _SEND_DATA_TCP_CMD;

    // Send data in chunks
    for (size_t chunk = 0; chunk < total_chunks; chunk++) {
        size_t offset = chunk * SOCKET_CHUNK_SIZE;
        size_t chunk_size = (offset + SOCKET_CHUNK_SIZE > len) ? (len - offset) : SOCKET_CHUNK_SIZE;

        const uint8_t *params[2] = { &sock_byte, buffer + offset };
        size_t param_lens[2] = { 1, chunk_size };
        uint8_t *responses[1];
        size_t response_lens[1];

        size_t num_resp = send_command_get_response(self, send_command, params, param_lens, 2, responses, response_lens, 1);

        if (num_resp > 0 && response_lens[0] > 0) {
            sent += responses[0][0];
            m_free(responses[0]);
        }
    }

    if (conn_mode == ADAFRUIT_ESP32SPI_UDP_MODE) {
        // UDP needs finalization
        if (sent != total_chunks) {
            mp_raise_ConnectionError(MP_ERROR_TEXT("Failed to write all chunks"));
        }

        const uint8_t *params[1] = { &sock_byte };
        size_t param_lens[1] = { 1 };
        uint8_t *responses[1];
        size_t response_lens[1];

        size_t num_resp = send_command_get_response(self, _SEND_UDP_DATA_CMD, params, param_lens, 1, responses, response_lens, 1);

        if (num_resp > 0) {
            uint8_t result = responses[0][0];
            m_free(responses[0]);
            if (result != 1) {
                mp_raise_ConnectionError(MP_ERROR_TEXT("Failed to send UDP data"));
            }
        }
    } else {
        // TCP verification
        if (sent != len) {
            mp_raise_ConnectionError(MP_ERROR_TEXT("Failed to send all bytes"));
        }

        const uint8_t *params[1] = { &sock_byte };
        size_t param_lens[1] = { 1 };
        uint8_t *responses[1];
        size_t response_lens[1];

        size_t num_resp = send_command_get_response(self, _DATA_SENT_TCP_CMD, params, param_lens, 1, responses, response_lens, 1);

        if (num_resp > 0) {
            uint8_t result = responses[0][0];
            m_free(responses[0]);
            if (result != 1) {
                mp_raise_ConnectionError(MP_ERROR_TEXT("Failed to verify data sent"));
            }
        }
    }
}

uint16_t common_hal_adafruit_esp32spi_esp_spicontrol_socket_available(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    uint8_t socket_num) {

    uint8_t sock_byte = socket_num;
    const uint8_t *params[1] = { &sock_byte };
    size_t param_lens[1] = { 1 };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _AVAIL_DATA_TCP_CMD, params, param_lens, 1, responses, response_lens, 1);

    if (num_resp > 0 && response_lens[0] >= 2) {
        uint16_t available = responses[0][0] | (responses[0][1] << 8);
        m_free(responses[0]);
        return available;
    }

    return 0;
}

size_t common_hal_adafruit_esp32spi_esp_spicontrol_socket_read(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    uint8_t socket_num,
    uint8_t *buffer,
    size_t size) {

    uint8_t sock_byte = socket_num;
    uint8_t size_bytes[2];
    size_bytes[0] = size & 0xFF;
    size_bytes[1] = (size >> 8) & 0xFF;

    const uint8_t *params[2] = { &sock_byte, size_bytes };
    size_t param_lens[2] = { 1, 2 };

    // Special handling for 16-bit length parameters
    // We need to modify send_command to handle 16-bit param lengths for this command
    // For now, use a simpler approach
    send_command(self, _GET_DATABUF_TCP_CMD, params, param_lens, 2);

    // Wait for response with 16-bit parameter length
    spi_begin_transaction(self);

    wait_spi_char(self, _START_CMD);
    check_data(self, _GET_DATABUF_TCP_CMD | _REPLY_FLAG);
    uint8_t num_responses = read_byte(self);

    size_t bytes_read = 0;
    if (num_responses > 0) {
        uint8_t param_len_high = read_byte(self);
        uint8_t param_len_low = read_byte(self);
        uint16_t param_len = (param_len_high << 8) | param_len_low;

        bytes_read = (param_len < size) ? param_len : size;
        common_hal_busio_spi_read(self->spi, buffer, bytes_read, 0xFF);

        // If there's more data than buffer, read and discard it
        for (size_t i = bytes_read; i < param_len; i++) {
            read_byte(self);
        }
    }

    check_data(self, _END_CMD);
    spi_end_transaction(self);

    return bytes_read;
}

void common_hal_adafruit_esp32spi_esp_spicontrol_socket_close(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    uint8_t socket_num) {

    uint8_t sock_byte = socket_num;
    const uint8_t *params[1] = { &sock_byte };
    size_t param_lens[1] = { 1 };
    uint8_t *responses[1];
    size_t response_lens[1];

    // Try to close, but don't fail if it errors
    send_command_get_response(self, _STOP_CLIENT_TCP_CMD, params, param_lens, 1, responses, response_lens, 1);

    if (socket_num == self->tls_socket) {
        self->tls_socket = -1;
    }

    // Free any responses
    for (size_t i = 0; i < 1; i++) {
        if (response_lens[i] > 0) {
            m_free(responses[i]);
        }
    }
}

// Network scanning
void common_hal_adafruit_esp32spi_esp_spicontrol_start_scan_networks(adafruit_esp32spi_esp_spicontrol_obj_t *self) {
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _START_SCAN_NETWORKS, NULL, NULL, 0, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_OSError_msg(MP_ERROR_TEXT("Failed to start AP scan"));
        }
    }
}

// Server operations
void common_hal_adafruit_esp32spi_esp_spicontrol_start_server(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    uint16_t port,
    uint8_t socket_num,
    adafruit_esp32spi_conn_mode_t conn_mode) {

    uint8_t port_bytes[2];
    port_bytes[0] = (port >> 8) & 0xFF;
    port_bytes[1] = port & 0xFF;

    uint8_t sock_byte = socket_num;
    uint8_t mode_byte = (uint8_t)conn_mode;

    const uint8_t *params[3] = { port_bytes, &sock_byte, &mode_byte };
    size_t param_lens[3] = { 2, 1, 1 };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _START_SERVER_TCP_CMD, params, param_lens, 3, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_OSError_msg(MP_ERROR_TEXT("Could not start server"));
        }
    }
}

uint8_t common_hal_adafruit_esp32spi_esp_spicontrol_server_state(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    uint8_t socket_num) {

    uint8_t sock_byte = socket_num;
    const uint8_t *params[1] = { &sock_byte };
    size_t param_lens[1] = { 1 };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _GET_STATE_TCP_CMD, params, param_lens, 1, responses, response_lens, 1);

    if (num_resp > 0 && response_lens[0] > 0) {
        uint8_t state = responses[0][0];
        m_free(responses[0]);
        return state;
    }

    return 0;
}

// GPIO operations
void common_hal_adafruit_esp32spi_esp_spicontrol_set_pin_mode(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    uint8_t pin,
    uint8_t mode) {

    const uint8_t *params[2] = { &pin, &mode };
    size_t param_lens[2] = { 1, 1 };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _SET_PIN_MODE_CMD, params, param_lens, 2, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_OSError_msg(MP_ERROR_TEXT("Failed to set pin mode"));
        }
    }
}

void common_hal_adafruit_esp32spi_esp_spicontrol_set_digital_write(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    uint8_t pin,
    bool value) {

    uint8_t val = value ? 1 : 0;
    const uint8_t *params[2] = { &pin, &val };
    size_t param_lens[2] = { 1, 1 };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _SET_DIGITAL_WRITE_CMD, params, param_lens, 2, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_OSError_msg(MP_ERROR_TEXT("Failed to write to pin"));
        }
    }
}

void common_hal_adafruit_esp32spi_esp_spicontrol_set_analog_write(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    uint8_t pin,
    uint8_t value) {

    const uint8_t *params[2] = { &pin, &value };
    size_t param_lens[2] = { 1, 1 };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _SET_ANALOG_WRITE_CMD, params, param_lens, 2, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_OSError_msg(MP_ERROR_TEXT("Failed to write to pin"));
        }
    }
}

bool common_hal_adafruit_esp32spi_esp_spicontrol_set_digital_read(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    uint8_t pin) {

    const uint8_t *params[1] = { &pin };
    size_t param_lens[1] = { 1 };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _SET_DIGITAL_READ_CMD, params, param_lens, 1, responses, response_lens, 1);

    if (num_resp > 0 && response_lens[0] > 0) {
        uint8_t value = responses[0][0];
        m_free(responses[0]);
        if (value == 0) {
            return false;
        } else if (value == 1) {
            return true;
        }
        mp_raise_OSError_msg(MP_ERROR_TEXT("Digital read response error"));
    }

    return false;
}

uint16_t common_hal_adafruit_esp32spi_esp_spicontrol_set_analog_read(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    uint8_t pin,
    uint8_t atten) {

    const uint8_t *params[2] = { &pin, &atten };
    size_t param_lens[2] = { 1, 1 };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _SET_ANALOG_READ_CMD, params, param_lens, 2, responses, response_lens, 1);

    if (num_resp > 0 && response_lens[0] >= 4) {
        int32_t value = responses[0][0] | (responses[0][1] << 8) | (responses[0][2] << 16) | (responses[0][3] << 24);
        m_free(responses[0]);
        if (value < 0) {
            mp_raise_ValueError(MP_ERROR_TEXT("Analog read error: invalid pin"));
        }
        return (uint16_t)(value * 16);  // Scale to 16-bit
    }

    return 0;
}

// Time operations
uint32_t common_hal_adafruit_esp32spi_esp_spicontrol_get_time(adafruit_esp32spi_esp_spicontrol_obj_t *self) {
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _GET_TIME, NULL, NULL, 0, responses, response_lens, 1);

    if (num_resp > 0 && response_lens[0] >= 4) {
        uint32_t timestamp = responses[0][0] | (responses[0][1] << 8) | (responses[0][2] << 16) | (responses[0][3] << 24);
        m_free(responses[0]);
        if (timestamp == 0) {
            mp_raise_OSError_msg(MP_ERROR_TEXT("get_time returned 0"));
        }
        return timestamp;
    }

    return 0;
}

// Debug operations
void common_hal_adafruit_esp32spi_esp_spicontrol_set_esp_debug(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    bool enabled) {

    uint8_t val = enabled ? 1 : 0;
    const uint8_t *params[1] = { &val };
    size_t param_lens[1] = { 1 };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _SET_DEBUG_CMD, params, param_lens, 1, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_OSError_msg(MP_ERROR_TEXT("Failed to set debug mode"));
        }
    }
}

// Network configuration methods
void common_hal_adafruit_esp32spi_esp_spicontrol_wifi_set_network(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    const uint8_t *ssid,
    size_t ssid_len) {

    const uint8_t *params[1] = { ssid };
    size_t param_lens[1] = { ssid_len };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _SET_NET_CMD, params, param_lens, 1, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_OSError_msg(MP_ERROR_TEXT("Failed to set network"));
        }
    }
}

void common_hal_adafruit_esp32spi_esp_spicontrol_wifi_set_passphrase(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    const uint8_t *ssid,
    size_t ssid_len,
    const uint8_t *passphrase,
    size_t passphrase_len) {

    const uint8_t *params[2] = { ssid, passphrase };
    size_t param_lens[2] = { ssid_len, passphrase_len };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _SET_PASSPHRASE_CMD, params, param_lens, 2, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_OSError_msg(MP_ERROR_TEXT("Failed to set passphrase"));
        }
    }
}

void common_hal_adafruit_esp32spi_esp_spicontrol_set_ip_config(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    const uint8_t *ip,
    const uint8_t *gateway,
    const uint8_t *mask) {

    uint8_t zero_byte = 0;
    const uint8_t *params[4] = { &zero_byte, ip, gateway, mask };
    size_t param_lens[4] = { 1, 4, 4, 4 };
    uint8_t *responses[1];
    size_t response_lens[1];

    send_command_get_response(self, _SET_IP_CONFIG, params, param_lens, 4, responses, response_lens, 1);

    if (response_lens[0] > 0) {
        m_free(responses[0]);
    }
}

void common_hal_adafruit_esp32spi_esp_spicontrol_set_dns_config(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    const uint8_t *dns1,
    const uint8_t *dns2) {

    uint8_t zero_byte = 0;
    const uint8_t *params[3] = { &zero_byte, dns1, dns2 };
    size_t param_lens[3] = { 1, 4, 4 };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _SET_DNS_CONFIG, params, param_lens, 3, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_OSError_msg(MP_ERROR_TEXT("Failed to set DNS"));
        }
    }
}

void common_hal_adafruit_esp32spi_esp_spicontrol_set_hostname(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    const char *hostname,
    size_t hostname_len) {

    const uint8_t *params[1] = { (const uint8_t *)hostname };
    size_t param_lens[1] = { hostname_len };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _SET_HOSTNAME, params, param_lens, 1, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_OSError_msg(MP_ERROR_TEXT("Failed to set hostname"));
        }
    }
}

// WPA2 Enterprise methods
void common_hal_adafruit_esp32spi_esp_spicontrol_wifi_set_entidentity(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    const uint8_t *ident,
    size_t ident_len) {

    const uint8_t *params[1] = { ident };
    size_t param_lens[1] = { ident_len };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _SET_ENT_IDENT_CMD, params, param_lens, 1, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_OSError_msg(MP_ERROR_TEXT("Failed to set enterprise identity"));
        }
    }
}

void common_hal_adafruit_esp32spi_esp_spicontrol_wifi_set_entusername(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    const uint8_t *username,
    size_t username_len) {

    const uint8_t *params[1] = { username };
    size_t param_lens[1] = { username_len };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _SET_ENT_UNAME_CMD, params, param_lens, 1, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_OSError_msg(MP_ERROR_TEXT("Failed to set enterprise username"));
        }
    }
}

void common_hal_adafruit_esp32spi_esp_spicontrol_wifi_set_entpassword(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    const uint8_t *password,
    size_t password_len) {

    const uint8_t *params[1] = { password };
    size_t param_lens[1] = { password_len };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _SET_ENT_PASSWD_CMD, params, param_lens, 1, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_OSError_msg(MP_ERROR_TEXT("Failed to set enterprise password"));
        }
    }
}

void common_hal_adafruit_esp32spi_esp_spicontrol_wifi_set_entenable(adafruit_esp32spi_esp_spicontrol_obj_t *self) {
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _SET_ENT_ENABLE_CMD, NULL, NULL, 0, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_OSError_msg(MP_ERROR_TEXT("Failed to enable enterprise mode"));
        }
    }
}

// TLS certificate methods
void common_hal_adafruit_esp32spi_esp_spicontrol_set_certificate(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    const uint8_t *certificate,
    size_t certificate_len) {

    const uint8_t *params[1] = { certificate };
    size_t param_lens[1] = { certificate_len };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _SET_CLI_CERT, params, param_lens, 1, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_OSError_msg(MP_ERROR_TEXT("Failed to set certificate"));
        }
    }
}

void common_hal_adafruit_esp32spi_esp_spicontrol_set_private_key(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    const uint8_t *private_key,
    size_t private_key_len) {

    const uint8_t *params[1] = { private_key };
    size_t param_lens[1] = { private_key_len };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _SET_PK, params, param_lens, 1, responses, response_lens, 1);

    if (num_resp > 0) {
        uint8_t result = responses[0][0];
        m_free(responses[0]);
        if (result != 1) {
            mp_raise_OSError_msg(MP_ERROR_TEXT("Failed to set private key"));
        }
    }
}

// Get remote data (IP and port for a socket connection)
void common_hal_adafruit_esp32spi_esp_spicontrol_get_remote_data(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    uint8_t socket_num,
    uint8_t *ip,
    uint16_t *port) {

    uint8_t sock_byte = socket_num;
    const uint8_t *params[1] = { &sock_byte };
    size_t param_lens[1] = { 1 };
    uint8_t *responses[2];
    size_t response_lens[2];

    size_t num_resp = send_command_get_response(self, _GET_REMOTE_DATA_CMD, params, param_lens, 1, responses, response_lens, 2);

    if (num_resp >= 2) {
        // First response is IP address
        if (response_lens[0] >= 4) {
            memcpy(ip, responses[0], 4);
        }
        // Second response is port
        if (response_lens[1] >= 2) {
            *port = responses[1][0] | (responses[1][1] << 8);
        }

        for (size_t i = 0; i < num_resp; i++) {
            m_free(responses[i]);
        }
    }
}

// Network class helper functions
void common_hal_adafruit_esp32spi_esp_spicontrol_get_curr_ssid(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    uint8_t *ssid,
    size_t *ssid_len) {

    const uint8_t param = 0xFF;
    const uint8_t *params[1] = { &param };
    size_t param_lens[1] = { 1 };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _GET_CURR_SSID_CMD, params, param_lens, 1, responses, response_lens, 1);

    if (num_resp > 0 && response_lens[0] > 0) {
        *ssid_len = response_lens[0];
        if (*ssid_len > 32) {
            *ssid_len = 32;  // Max SSID length
        }
        memcpy(ssid, responses[0], *ssid_len);
        m_free(responses[0]);
    } else {
        *ssid_len = 0;
    }
}

void common_hal_adafruit_esp32spi_esp_spicontrol_get_curr_bssid(
    adafruit_esp32spi_esp_spicontrol_obj_t *self,
    uint8_t *bssid) {

    const uint8_t param = 0xFF;
    const uint8_t *params[1] = { &param };
    size_t param_lens[1] = { 1 };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _GET_CURR_BSSID_CMD, params, param_lens, 1, responses, response_lens, 1);

    if (num_resp > 0 && response_lens[0] >= 6) {
        memcpy(bssid, responses[0], 6);
        m_free(responses[0]);
    } else {
        memset(bssid, 0, 6);
    }
}

int32_t common_hal_adafruit_esp32spi_esp_spicontrol_get_curr_rssi(
    adafruit_esp32spi_esp_spicontrol_obj_t *self) {

    const uint8_t param = 0xFF;
    const uint8_t *params[1] = { &param };
    size_t param_lens[1] = { 1 };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _GET_CURR_RSSI_CMD, params, param_lens, 1, responses, response_lens, 1);

    int32_t rssi = 0;
    if (num_resp > 0 && response_lens[0] >= 4) {
        // Unpack as little-endian int32
        rssi = (int32_t)(responses[0][0] | (responses[0][1] << 8) | (responses[0][2] << 16) | (responses[0][3] << 24));
        m_free(responses[0]);
    }

    return rssi;
}

uint8_t common_hal_adafruit_esp32spi_esp_spicontrol_get_curr_enct(
    adafruit_esp32spi_esp_spicontrol_obj_t *self) {

    const uint8_t param = 0xFF;
    const uint8_t *params[1] = { &param };
    size_t param_lens[1] = { 1 };
    uint8_t *responses[1];
    size_t response_lens[1];

    size_t num_resp = send_command_get_response(self, _GET_CURR_ENCT_CMD, params, param_lens, 1, responses, response_lens, 1);

    uint8_t enct = 0;
    if (num_resp > 0 && response_lens[0] > 0) {
        enct = responses[0][0];
        m_free(responses[0]);
    }

    return enct;
}
