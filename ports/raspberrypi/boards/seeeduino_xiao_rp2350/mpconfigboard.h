// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2024 Scott Shawcroft for Adafruit Industries
//
// SPDX-License-Identifier: MIT

#pragma once

#define MICROPY_HW_BOARD_NAME "Seeeduino XIAO RP2350"
#define MICROPY_HW_MCU_NAME "rp2350"

#define MICROPY_HW_NEOPIXEL (&pin_GPIO22)
#define CIRCUITPY_STATUS_LED_POWER (&pin_GPIO23)

#define DEFAULT_I2C_BUS_SCL (&pin_GPIO7)
#define DEFAULT_I2C_BUS_SDA (&pin_GPIO6)

#define DEFAULT_SPI_BUS_SCK (&pin_GPIO2)
#define DEFAULT_SPI_BUS_MOSI (&pin_GPIO3)
#define DEFAULT_SPI_BUS_MISO (&pin_GPIO4)

#define DEFAULT_UART_BUS_RX (&pin_GPIO1)
#define DEFAULT_UART_BUS_TX (&pin_GPIO0)
