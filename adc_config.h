
 #ifndef ADC_CONFIG_H
#define ADC_CONFIG_H

#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         4 

#include "SEGGER_RTT.h"
#include "nrf.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "nrf_drv_adc.h"
#include "nordic_common.h"
#include "boards.h"
#define NRF_LOG_MODULE_NAME "APP"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "app_util_platform.h"

void adc_config(void);

void adc_event_handler(nrf_drv_adc_evt_t const * p_event);
unsigned short adc_read(unsigned int adc_in_mask);
#endif

