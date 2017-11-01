#ifndef BLE_CONFIG_H
#define BLE_CONFIG_H

#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         4 


#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "app_button.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "SEGGER_RTT.h"

#include "pca10028.h"
#include <stdint.h>
#include <string.h>



 void gap_params_init(char * device_name);
 void services_init(void);
 void on_conn_params_evt(ble_conn_params_evt_t * p_evt);
 void conn_params_error_handler(uint32_t nrf_error);
 void conn_params_init(void);
 void on_adv_evt(ble_adv_evt_t ble_adv_evt);
 void on_ble_evt(ble_evt_t * p_ble_evt);
 void ble_evt_dispatch(ble_evt_t * p_ble_evt);
 void ble_stack_init(void);
 void advertising_init(void);
 void sending_msg(char* msg);
 void on_off_advertising(void);
 char **str_split (char *s, const char *ct);

#endif

