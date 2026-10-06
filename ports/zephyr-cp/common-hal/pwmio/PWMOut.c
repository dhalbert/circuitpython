// This file is part of the CircuitPython project: https://circuitpython.org
//
// SPDX-FileCopyrightText: Copyright (c) 2026 Dan Halbert for Adafruit Industries LLC
//
// SPDX-License-Identifier: MIT

// pwmio on Zephyr's PWM API. The iobroker module picks the PWM device and
// channel for the pin and adjusts the frequency to a period the hardware
// can produce; channels that share a period (an nRF instance's outputs) are
// grouped there, so nothing here is SoC-specific.

#include <errno.h>

#include <iobroker/iobroker.h>
#include <zephyr/device.h>
#include <zephyr/drivers/pwm.h>

#include "py/runtime.h"

#include "shared-bindings/pwmio/PWMOut.h"

static uint32_t pulse_cycles(uint32_t period_cycles, uint16_t duty) {
    return (uint32_t)(((uint64_t)period_cycles * duty) / 0xffff);
}

// Program the channel's period and the stored duty cycle.
static int set_period_and_duty(pwmio_pwmout_obj_t *self) {
    return pwm_set_cycles(self->dev, self->channel, self->period_cycles,
        pulse_cycles(self->period_cycles, self->duty_cycle), 0);
}

pwmout_result_t common_hal_pwmio_pwmout_construct(pwmio_pwmout_obj_t *self,
    const mcu_pin_obj_t *pin, uint16_t duty, uint32_t frequency,
    bool variable_frequency) {
    // Deinited until construction succeeds.
    self->dev = NULL;
    const struct device *dev;
    uint32_t channel;
    uint32_t period_cycles;
    int ret = iobroker_pwm_channel_allocate(pin->package_pin, frequency,
        variable_frequency, &dev, &channel, &period_cycles);
    switch (ret) {
        case 0:
            break;
        case -ERANGE:
            return PWMOUT_INVALID_FREQUENCY;
        case -ENODEV:
            return PWMOUT_INTERNAL_RESOURCES_IN_USE;
        case -EINVAL:
        case -EBUSY:
        case -ENOSYS:
            return PWMOUT_INVALID_PIN;
        default:
            return PWMOUT_INITIALIZATION_ERROR;
    }

    // The first channel of a group initializes the device; an existing
    // group reports -EALREADY.
    ret = device_init(dev);
    if (ret < 0 && ret != -EALREADY) {
        (void)iobroker_pwm_channel_release(dev, channel);
        return PWMOUT_INITIALIZATION_ERROR;
    }

    self->pin = pin;
    self->dev = dev;
    self->channel = channel;
    self->period_cycles = period_cycles;
    self->duty_cycle = duty;
    self->variable_frequency = variable_frequency;
    if (set_period_and_duty(self) < 0) {
        (void)iobroker_pwm_channel_release(dev, channel);
        self->dev = NULL;
        return PWMOUT_INITIALIZATION_ERROR;
    }
    return PWMOUT_OK;
}

bool common_hal_pwmio_pwmout_deinited(pwmio_pwmout_obj_t *self) {
    return self->dev == NULL;
}

void common_hal_pwmio_pwmout_deinit(pwmio_pwmout_obj_t *self) {
    if (common_hal_pwmio_pwmout_deinited(self)) {
        return;
    }
    // Stop the output, then hand the channel back.
    (void)pwm_set_cycles(self->dev, self->channel, self->period_cycles, 0, 0);
    (void)iobroker_pwm_channel_release(self->dev, self->channel);
    self->dev = NULL;
    self->pin = NULL;
}

void common_hal_pwmio_pwmout_set_duty_cycle(pwmio_pwmout_obj_t *self, uint16_t duty) {
    self->duty_cycle = duty;
    (void)set_period_and_duty(self);
}

uint16_t common_hal_pwmio_pwmout_get_duty_cycle(pwmio_pwmout_obj_t *self) {
    return self->duty_cycle;
}

void common_hal_pwmio_pwmout_set_frequency(pwmio_pwmout_obj_t *self, uint32_t frequency) {
    // Only used when variable_frequency=True, so the
    // channel's group is exclusive and no other channel shares the period.
    uint32_t period_cycles;
    if (iobroker_pwm_period_cycles(self->dev, frequency, &period_cycles) < 0) {
        common_hal_pwmio_pwmout_raise_error(PWMOUT_INVALID_FREQUENCY);
    }
    self->period_cycles = period_cycles;
    (void)set_period_and_duty(self);
}

uint32_t common_hal_pwmio_pwmout_get_frequency(pwmio_pwmout_obj_t *self) {
    // The frequency the hardware actually produces.
    uint64_t cycles_per_sec;
    if (pwm_get_cycles_per_sec(self->dev, self->channel, &cycles_per_sec) < 0) {
        return 0;
    }
    return (uint32_t)((cycles_per_sec + self->period_cycles / 2) / self->period_cycles);
}

bool common_hal_pwmio_pwmout_get_variable_frequency(pwmio_pwmout_obj_t *self) {
    return self->variable_frequency;
}

const mcu_pin_obj_t *common_hal_pwmio_pwmout_get_pin(pwmio_pwmout_obj_t *self) {
    return self->pin;
}

void common_hal_pwmio_pwmout_reset_ok(pwmio_pwmout_obj_t *self) {
    // CIRCUITPY_BULK_RESET is off on this port: nothing is kept across
    // resets, so there is nothing to undo.
    (void)self;
}
