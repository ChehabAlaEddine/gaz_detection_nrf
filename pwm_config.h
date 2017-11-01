
#ifndef PWM_CONFIG_H
#define PWM_CONFIG_H



#include <stdbool.h>
#include <stdint.h>
#include "nrf.h"
#include "app_error.h"
#include "bsp.h"
#include "nrf_delay.h"
#include "app_pwm.h"

void pwm_ready_callback(uint32_t pwm_id);
void buzzer_on();
void buzzer_off();
void pwm_init(uint32_t freq ,uint8_t  pin);


#endif

