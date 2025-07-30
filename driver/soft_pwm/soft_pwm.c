/**
 * @file soft_pwm.c
 * @brief Generate pwm waves on gpios by software
 * @author Ellu (ellu.grif@gmail.com)
 * @version 1.0.0
 * @date 2025-01-10
 *
 * THINK DIFFERENTLY
 */

#include "soft_pwm.h"

// Private Defines --------------------------

// Private Typedefs -------------------------

// Private Macros ---------------------------

// Private Variables ------------------------

// Public Variables -------------------------

// Private Functions ------------------------

// Public Functions -------------------------

void soft_pwm_runner(soft_pwm_t pwm_devs[], uint32_t count) {
    if (!pwm_devs || !count)
        return;

    while (count--) {
        if (!pwm_devs->reload || !pwm_devs->active || !pwm_devs->compare) {
            HAL_GPIO_WritePin(pwm_devs->port, pwm_devs->pin,
                              pwm_devs->invert ? GPIO_PIN_SET : GPIO_PIN_RESET);
            continue;
        }

        if (unlikely(pwm_devs->down_count)) {
            pwm_devs->count = (pwm_devs->count - 1) % pwm_devs->reload;
            if (pwm_devs->count == pwm_devs->compare) {
                HAL_GPIO_WritePin(
                    pwm_devs->port, pwm_devs->pin,
                    pwm_devs->invert ? GPIO_PIN_RESET : GPIO_PIN_SET);
            } else if (pwm_devs->count == pwm_devs->reload - 1) {
                HAL_GPIO_WritePin(
                    pwm_devs->port, pwm_devs->pin,
                    pwm_devs->invert ? GPIO_PIN_SET : GPIO_PIN_RESET);
            }
        } else {
            pwm_devs->count = (pwm_devs->count + 1) % pwm_devs->reload;
            if (pwm_devs->count == pwm_devs->compare + 1) {
                HAL_GPIO_WritePin(
                    pwm_devs->port, pwm_devs->pin,
                    pwm_devs->invert ? GPIO_PIN_SET : GPIO_PIN_RESET);
            } else if (pwm_devs->count == 0) {
                HAL_GPIO_WritePin(
                    pwm_devs->port, pwm_devs->pin,
                    pwm_devs->invert ? GPIO_PIN_RESET : GPIO_PIN_SET);
            }
        }

        pwm_devs++;
    }
}

void soft_pwm_set_freq(soft_pwm_t* pwm_dev, float freq, uint32_t runner_freq) {
    if (!pwm_dev || freq <= 0 || runner_freq <= 0)
        return;

    // Calculate reload value based on frequency
    uint32_t reload = (uint32_t)(runner_freq / freq);
    pwm_dev->reload = reload;
    if (pwm_dev->count > reload)
        pwm_dev->count = reload;
    if (pwm_dev->compare > reload)
        pwm_dev->compare = reload;
}

void soft_pwm_set_duty(soft_pwm_t* pwm_dev, float duty) {
    if (!pwm_dev)
        return;

    if (duty < 0)
        duty = 0;
    else if (duty > 100)
        duty = 100;

    // Convert percentage to compare value
    uint32_t compare = (uint32_t)(pwm_dev->reload * duty / 100.0f);
    pwm_dev->compare = compare;
    if (pwm_dev->count > compare)
        HAL_GPIO_WritePin(pwm_dev->port, pwm_dev->pin,
                          pwm_dev->invert ? GPIO_PIN_SET : GPIO_PIN_RESET);
    else
        HAL_GPIO_WritePin(pwm_dev->port, pwm_dev->pin,
                          pwm_dev->invert ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

// Source Code End --------------------------
