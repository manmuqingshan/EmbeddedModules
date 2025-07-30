/**
 * @file soft_pwm.h
 * @brief Generate pwm waves on gpios by software
 * @author Ellu (ellu.grif@gmail.com)
 * @date 2025-01-10
 *
 * THINK DIFFERENTLY
 */

#ifndef __SOFT_PWM_H__
#define __SOFT_PWM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "modules.h"
#include "stdbool.h"

// Public Defines ---------------------------

typedef struct {
    GPIO_TypeDef* port;
    uint16_t pin;
    uint32_t reload;
    uint32_t compare;
    uint32_t count;
    uint8_t active : 1;
    uint8_t invert : 1;
    uint8_t down_count : 1;
} soft_pwm_t;

// Public Typedefs --------------------------

// Public Macros ----------------------------

// Exported Variables -----------------------

/**
 * @brief Run the software PWM devices
 * @param  pwm_devs       Array of PWM devices to run
 * @param  count          Number of PWM devices in the array
 * @note Add (or minus) the count value by 1 each time this function is called
 */
void soft_pwm_runner(soft_pwm_t pwm_devs[], uint32_t count);

/**
 * @brief Set PWM reload value
 * @param  pwm_dev        PWM device pointer
 * @param  reload         PWM period value
 */
static inline void soft_pwm_set_reload(soft_pwm_t* pwm_dev, uint32_t reload) {
    pwm_dev->reload = reload;
}

/**
 * @brief Set PWM compare value
 * @param  pwm_dev        PWM device pointer
 * @param  compare        PWM compare value (should be <= reload)
 */
static inline void soft_pwm_set_compare(soft_pwm_t* pwm_dev, uint32_t compare) {
    if (compare > pwm_dev->reload)
        compare = pwm_dev->reload;
    pwm_dev->compare = compare;
}

/**
 * @brief Set PWM count value
 * @param  pwm_dev        PWM device pointer
 * @param  count          PWM duty cycle value (should be <= reload)
 */
static inline void soft_pwm_set_count(soft_pwm_t* pwm_dev, uint32_t count) {
    if (count > pwm_dev->reload)
        count = pwm_dev->reload;
    pwm_dev->count = count;
}

/**
 * @brief Set PWM active state
 * @param  pwm_dev        PWM device pointer
 * @param  active         PWM active state
 */
static inline void soft_pwm_set_active(soft_pwm_t* pwm_dev, bool active) {
    pwm_dev->active = active;
}

/**
 * @brief Set PWM invert state
 * @param  pwm_dev        PWM device pointer
 * @param  invert         PWM invert state
 * @note Non-invert: count<=compare -> high, count>compare -> low,
 *       Invert: count<=compare -> low, count>compare -> high
 */
static inline void soft_pwm_set_invert(soft_pwm_t* pwm_dev, bool invert) {
    pwm_dev->invert = invert;
}

/**
 * @brief Set PWM down count state (count--)
 * @param  pwm_dev        PWM device pointer
 * @param  down_count     PWM down count state
 */
static inline void soft_pwm_set_down_count(soft_pwm_t* pwm_dev,
                                           bool down_count) {
    pwm_dev->down_count = down_count;
}

/**
 * @brief Set PWM frequency
 * @param  pwm_dev        PWM device pointer
 * @param  freq           Target frequency in Hz
 * @param  runner_freq    Runner function call frequency in Hz
 */
void soft_pwm_set_freq(soft_pwm_t* pwm_dev, float freq, uint32_t runner_freq);

/**
 * @brief Set PWM duty cycle
 * @param  pwm_dev        PWM device pointer
 * @param  duty           Duty cycle (0.0 - 100.0)
 */
void soft_pwm_set_duty(soft_pwm_t* pwm_dev, float duty);

// Exported Functions -----------------------

#ifdef __cplusplus
}
#endif

#endif /* __SOFT_PWM__ */
