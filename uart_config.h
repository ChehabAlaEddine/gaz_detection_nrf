#ifndef UART_CONFIG__H
#define UART_CONFIG__H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "app_uart.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "nrf.h"
//#include "pca10028.h"

#include "SEGGER_RTT.h"




#define RT_IDEL 6
#define RT_SUCCESS 0
#define RT_ERROR 5
#define RT_NOT_FOUND 1
#define RT_TIME_OUT 2
#define RT_FULL 3
#define RT_HTTP_ERROR 4
#define RT_HTTP_NOT_FINISH 7

void sms_listener(void);
void uart_init(void);
void uart_put_string(const char * msg);
int uart_get_string(char * data_array , uint16_t lenght);
void uart_put_string_len(const char * msg, uint8_t len);
int wait_uart_event(uint16_t time_out);

int waitForResp_repeat(const char * resp, const char* resp_busy , uint16_t time_out);
int waitForResp(const char * resp, uint16_t timeout);
int waitForResp__(const char * resp1, const char * resp2,const char * error, uint16_t time_out);
int waitForResp___(const char * resp1, const char * resp2,const char * error1,const char * error2, uint16_t time_out, bool uart_event);
	
int sendCmdAndWaitForResp(const char* cmd, const char *resp, uint16_t timeout);
int sendCmdAndWaitForResp_handler(const char*cmd, const char *resp, uint16_t timeout);
#endif //UART_CONFIG__H


