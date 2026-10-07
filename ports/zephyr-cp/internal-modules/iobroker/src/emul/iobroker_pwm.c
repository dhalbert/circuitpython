// Emulated PWM channel allocation for the iobroker module (native_sim).
//
// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2026 Dan Halbert for Adafruit Industries LLC
//
// SPDX-License-Identifier: MIT

// The emulated PWM controller (adafruit,pwm-emul, see the pwm_emul module)
// has one channel per GPIO: the channel a pad drives is the pad's global
// GPIO number, as with the emulated ADC and DAC. Channels that share a
// period form a group; the controller has a pool of them (the groups
// property), and any channel can join any group, without a channel limit.
// So this models only the pwmio rules every SoC shares, not any SoC's
// layout: a fixed-frequency request joins a group running at the same
// period, a variable-frequency request takes a group of its own, and with
// no usable group left the request fails. The pad is claimed as a GPIO,
// which makes it busy for every other allocation.

#include <errno.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

#include "iobroker_internal.h"

LOG_MODULE_DECLARE(iobroker, CONFIG_LOG_DEFAULT_LEVEL);

#define EMUL_PWM_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(adafruit_pwm_emul)
#define EMUL_PWM_CHANNELS DT_PROP(EMUL_PWM_NODE, channels)
#define EMUL_PWM_GROUPS DT_PROP(EMUL_PWM_NODE, groups)

// Shortest period accepted, in clock cycles: a period needs a high and a low
// part.
#define EMUL_PWM_MIN_PERIOD 2U

// Period groups. A group is free while it has no users.
typedef struct {
    uint32_t period_cycles;
    // One variable-frequency channel holds the group; nobody may join.
    bool exclusive;
    uint16_t users;
} emul_pwm_group_t;

static emul_pwm_group_t groups[EMUL_PWM_GROUPS];

// The group each channel belongs to, while the channel is allocated.
static uint8_t channel_group[EMUL_PWM_CHANNELS];
BUILD_ASSERT(EMUL_PWM_GROUPS <= UINT8_MAX, "group index must fit channel_group[]");

// Channels currently allocated, so release only drops claims it made.
static uint32_t allocated[DIV_ROUND_UP(EMUL_PWM_CHANNELS, 32)];

// Pick the group for a request: a running group at the same period that
// nobody holds exclusively, unless the request is exclusive itself, or else
// a free group. Returns the group index, or -1 when none is usable.
static int emul_pwm_group_pick(uint32_t period_cycles, bool exclusive) {
    if (!exclusive) {
        for (int i = 0; i < EMUL_PWM_GROUPS; i++) {
            if (groups[i].users > 0 && !groups[i].exclusive &&
                groups[i].period_cycles == period_cycles) {
                return i;
            }
        }
    }
    for (int i = 0; i < EMUL_PWM_GROUPS; i++) {
        if (groups[i].users == 0) {
            return i;
        }
    }
    return -1;
}

static const struct device *emul_pwm_device(void) {
    return DEVICE_DT_GET(EMUL_PWM_NODE);
}

// Adjust a frequency to a period of the clock by truncation, so the actual
// frequency is the same or slightly higher.
static int emul_pwm_frequency_period(uint32_t frequency, uint32_t *period_cycles_out) {
    uint64_t cycles_per_sec;
    if (frequency == 0 || pwm_get_cycles_per_sec(emul_pwm_device(), 0, &cycles_per_sec) < 0) {
        return -ERANGE;
    }
    uint64_t period = cycles_per_sec / frequency;
    if (period < EMUL_PWM_MIN_PERIOD || period > UINT32_MAX) {
        return -ERANGE;
    }
    *period_cycles_out = (uint32_t)period;
    return 0;
}

int iobroker_pwm_period_cycles(const struct device *dev, uint32_t frequency,
    uint32_t *period_cycles_out) {
    if (dev != emul_pwm_device()) {
        return -EINVAL;
    }
    return emul_pwm_frequency_period(frequency, period_cycles_out);
}

int iobroker_pwm_channel_allocate(package_pin_t pin, uint32_t frequency,
    bool exclusive, const struct device **dev_out, uint32_t *channel_out,
    uint32_t *period_cycles_out) {
    // An out-of-range frequency is reported first, ahead of pin and busy
    // errors.
    uint32_t period_cycles;
    if (emul_pwm_frequency_period(frequency, &period_cycles) < 0) {
        return -ERANGE;
    }
    uint16_t soc_pad;
    uint16_t gpio_pad;
    if (pin == IOBROKER_NO_PIN || iobroker_package_pin_soc_pad(pin, &soc_pad) < 0 ||
        iobroker_pad_gpio(soc_pad, &gpio_pad) < 0 || gpio_pad >= EMUL_PWM_CHANNELS) {
        LOG_WRN("pwm channel allocate: package pin %u has no emulated PWM channel",
            (unsigned)pin);
        return -EINVAL;
    }
    int group = emul_pwm_group_pick(period_cycles, exclusive);
    if (group < 0) {
        LOG_WRN("pwm channel allocate: no usable period group for package pin %u",
            (unsigned)pin);
        return -ENODEV;
    }
    const struct device *port;
    gpio_pin_t number;
    int ret = iobroker_gpio_allocate(pin, &port, &number);
    if (ret < 0) {
        return ret;
    }
    if (groups[group].users == 0) {
        groups[group].period_cycles = period_cycles;
        groups[group].exclusive = exclusive;
    }
    groups[group].users++;
    channel_group[gpio_pad] = (uint8_t)group;
    allocated[gpio_pad / 32] |= BIT(gpio_pad % 32);
    *dev_out = emul_pwm_device();
    *channel_out = gpio_pad;
    *period_cycles_out = period_cycles;
    LOG_INF("pwm channel allocate: package pin %u -> %s channel %u, group %d",
        (unsigned)pin, (*dev_out)->name, (unsigned)gpio_pad, group);
    return 0;
}

bool iobroker_pwm_channel_release(const struct device *dev, uint32_t channel) {
    if (dev != emul_pwm_device() || channel >= EMUL_PWM_CHANNELS ||
        !(allocated[channel / 32] & BIT(channel % 32))) {
        LOG_WRN("pwm channel release: %s channel %u is not allocated",
            dev == NULL ? "(null)" : dev->name, (unsigned)channel);
        return false;
    }
    allocated[channel / 32] &= ~BIT(channel % 32);
    groups[channel_group[channel]].users--;
    const struct device *port;
    gpio_pin_t number;
    if (iobroker_gpio_split((uint16_t)channel, &port, &number) == 0) {
        (void)iobroker_gpio_release(port, number);
    }
    return true;
}
