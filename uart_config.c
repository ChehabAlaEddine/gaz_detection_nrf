#include "uart_config.h"
#include "SEGGER_RTT_Conf.h"
//#include "timer_config.h"

#define UART_TX_BUF_SIZE 16                             /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE 512                          /**< UART RX buffer size. */


uint8_t op_not_index = 0;

extern void HardFault_Handler(void);
extern void power_manage(void);
extern char* recv;
//void power_manage(void);

#define DATA_SIZE 300
char recieve[DATA_SIZE];
char res_1[10];
char res_2[10];
char err_1[10];
char err_2[10];

volatile uint16_t indexx = 0; 
volatile uint8_t uart_data = 0;
volatile bool uart_data_found = false;
volatile bool uart_new_command = false;


bool uart_event = false;

#define RESP_TYPE_1_0 1  // success: 1 error:0
#define RESP_TYPE_1_1 2  // success: 1 error:1
#define RESP_TYPE_1_2 3  // success: 1 error:2
#define RESP_TYPE_2_1 4  // success: 2 error:1
#define RESP_TYPE_2_2 5  // success: 2 error:2

volatile uint8_t resp_type = RESP_TYPE_1_0;


volatile bool is_uart_timer_start = false;

void uart_handle(app_uart_evt_t *p_event) {
//    uint32_t       err_code;
		switch (p_event -> evt_type) {
				case APP_UART_DATA_READY:

						if (uart_event) {

								if (uart_new_command) {
										SEGGER_RTT_printf(0, " uart_new_command \n");
										memset((char *)recieve, '\0', indexx);
										indexx = 0;
										uart_new_command = false;
								}
								if (indexx == DATA_SIZE) {
										SEGGER_RTT_printf(0, "\n****** uart full *** %s\n",recieve);
										//SEGGER_RTT_printf(0,"%s\n",recieve);	
										memset((char *)recieve, '\0', indexx);
										indexx = 0;
								}
								
								

								app_uart_get((uint8_t *) & recieve[indexx++]);

								if (recieve[indexx-1] == '\n') {
										SEGGER_RTT_printf(0, "\n!!!!!!!!!!!!!!!!! %s\n",recieve);
										//SEGGER_RTT_printf(0,"%s\n",recieve);	
								}
								
								if (!uart_data_found) {

										switch (resp_type) {

												case RESP_TYPE_1_0:
														if (strstr(recieve, res_1) != NULL) {
																uart_data = RT_SUCCESS;
																uart_data_found = true;
															SEGGER_RTT_printf(0, "\n!!!!!!!!!!!!!!!!! %s\n",recieve);
														}
														break;

												case RESP_TYPE_1_1:
														if (strstr(recieve, res_1) != NULL) {
																uart_data = RT_SUCCESS;
																uart_data_found = true;
														} else if (strstr(recieve, err_1) != NULL) {
																uart_data = RT_ERROR;
																uart_data_found = true;
														}
														break;

												case RESP_TYPE_1_2:
														if (strstr(recieve, res_1) != NULL) {
																uart_data = RT_SUCCESS;
																uart_data_found = true;
														} else if (strstr(recieve, err_1) != NULL || strstr(recieve, err_2) != NULL) {
																uart_data = RT_ERROR;
																uart_data_found = true;
														}
														break;


												case RESP_TYPE_2_1:
														if (strstr(recieve, res_1) != NULL || strstr(recieve, res_2) != NULL) {
																uart_data = RT_SUCCESS;
																uart_data_found = true;
														} else if (strstr(recieve, err_1) != NULL) {
																uart_data = RT_ERROR;
																uart_data_found = true;
														}
														break;


												case RESP_TYPE_2_2:
														if (strstr(recieve, res_1) != NULL || strstr(recieve, res_2) != NULL) {
																uart_data = RT_SUCCESS;
																uart_data_found = true;
														} else if (strstr(recieve, err_1) != NULL || strstr(recieve, err_2) != NULL) {
																uart_data = RT_ERROR;
																uart_data_found = true;
														}else if (strstr(recieve, "Connection") != NULL && strstr(recieve, "200 OK") == NULL) {
																uart_data = RT_ERROR;
																uart_data_found = true;
														}
														break;

										}
										if (uart_data_found)
												SEGGER_RTT_printf(0, "resp_type %d \n%s \n", resp_type, recieve);

								}

						}
						break;


				case APP_UART_COMMUNICATION_ERROR:
						SEGGER_RTT_printf(0, "APP_UART_COMMUNICATION_ERROR \n");
				//	APP_ERROR_HANDLER(p_event->data.error_communication);
				//	NVIC_SystemReset();
						
				    HardFault_Handler();
						break;

				case APP_UART_FIFO_ERROR:
						SEGGER_RTT_printf(0, "APP_UART_FIFO_ERROR\n");
						APP_ERROR_HANDLER(p_event -> data.error_code);
						break;

				case APP_UART_DATA:
						SEGGER_RTT_printf(0, "APP_UART_DATA\n");
						break;
				default:
						break;


		}


}


/**
 * @brief Function for handling the Battery measurement timer timeout.
 * @details This function will be called each time the battery level measurement timer expires.
 * @param[in] p_context   Pointer used for passing some arbitrary information (context) from the
 * app_start_timer() call to the timeout handler.
 */

void uart_timeout_timer_handler(void *p_context) {

		SEGGER_RTT_printf(0, "&&&&& uart_timeout_timer_handler &&&&\n");
		nrf_delay_ms(5);
		is_uart_timer_start = false;
}


void uart_init(void) {

		uint32_t err_code;
 const app_uart_comm_params_t comm_params =
						{
										25,//5,//12,//RX_PIN_NUMBER,
										28,//7,//13,//TX_PIN_NUMBER,
										14,
										15,
										APP_UART_FLOW_CONTROL_DISABLED,
										false,
										UART_BAUDRATE_BAUDRATE_Baud9600
						};

		APP_UART_FIFO_INIT( & comm_params,
						UART_RX_BUF_SIZE,
						UART_TX_BUF_SIZE,
						uart_handle,
						APP_IRQ_PRIORITY_LOW,
						err_code);

		APP_ERROR_CHECK(err_code);
}




int waitForResp_repeat(const char *resp, const char *resp_busy, uint16_t time_out) {
		uart_new_command = true;
		uart_event = true;
		uart_data = RT_IDEL;
		resp_type = RESP_TYPE_1_1;
		memcpy(res_1, resp, 10);
		memcpy(err_1, resp_busy, 10);

		return wait_uart_event(time_out);

//    char receive[BUFFER_SIZE_UP];
//		uint8_t time=0;
//	repeat:
//	  time++;
//	  nrf_delay_ms(100);
//	  memset(receive,0,BUFFER_SIZE_UP);
//    uart_get_string((char*)receive,BUFFER_SIZE_UP);
//	  SEGGER_RTT_printf(0,"\n waitForResp_repeat %s ",receive);
//	
//	if(time>time_out){
//		SEGGER_RTT_printf(0,"\n waitForResp_repeat time out");
//	return RT_TIME_OUT;
//	}
//		
//	
//	  if(strstr(receive,resp)!=NULL){
//			SEGGER_RTT_printf(0,"\n waitForResp_repeat response is  found");
//			return RT_SUCCESS;
//		}
//		else {
//			SEGGER_RTT_printf(0,"\n waitForResp_repeat response is  not found");
//			if(strstr(receive,resp_busy)!=NULL ){
//				goto repeat;
//			}
//		}
//    return RT_NOT_FOUND;
}


int waitForResp(const char *resp, uint16_t timeout) {
	//SEGGER_RTT_printf(0,"\nwaitForResp have been called with resp:%s",resp);	
    char receive[BUFFER_SIZE_UP];
	  nrf_delay_ms(timeout);
	  memset(receive,0,BUFFER_SIZE_UP);
    uart_get_string((char*)receive,BUFFER_SIZE_UP);
	SEGGER_RTT_printf(0,"\nrecieve %s  ::",receive);
   recv=receive;	 
	if(strstr(receive,resp)!=NULL){
			SEGGER_RTT_printf(0,"\n the response %s is  found in %s **waitForResp**", resp, receive);
			return RT_SUCCESS;
		}
		else {
			SEGGER_RTT_printf(0,"\n the response is  not found in %s **waitForResp**",receive);
			return RT_ERROR;
		}

}

int waitForResp__(const char *resp1, const char *resp2,const char *error, uint16_t time_out) {
		uart_new_command = true;
		uart_event = true;
		uart_data = RT_IDEL;
		resp_type = RESP_TYPE_2_1;
		memcpy(res_1, resp1, 10);
		memcpy(res_2, resp2, 10);
		memcpy(err_1, error, 10);

		return wait_uart_event(time_out);

//	  char receive[BUFFER_SIZE_UP];
//    uint8_t time=0;
//	  uint8_t response;
//	repeat:
//	  
//	  nrf_delay_ms(100);
//    time++;
//	 	if(time>time_out){
//		   SEGGER_RTT_printf(0,"\n waitForResp_repeat time out");
//	     return RT_TIME_OUT;
//		}
//	  memset(receive,0,BUFFER_SIZE_UP);
//    response=uart_get_string((char*)receive,BUFFER_SIZE_UP);
//	  SEGGER_RTT_printf(0,"\nrecieve %s ",receive);
//	  if(strstr(receive,resp1)!=NULL || strstr(receive,resp2)!=NULL){
//			//SEGGER_RTT_printf(0,"\n waitForResp__ response is  found");
//			if(response==RT_FULL)
//				goto repeat;
//			return RT_SUCCESS;
//		}
//		else {
//				//SEGGER_RTT_printf(0,"\n the response is  not found");
//			if(response==RT_FULL)
//				goto repeat;
//			
//			if(strstr(receive,error)!=NULL){
//				return RT_ERROR;
//			}
//			goto repeat;
//		}


}

int waitForResp___(const char *resp1, const char *resp2,const char *error1,const char *error2, uint16_t time_out, bool uart_eventtt) {
		if (uart_eventtt) {
				uart_new_command = true;
				uart_event = true;
				uart_data = RT_IDEL;
				resp_type = RESP_TYPE_2_2;
				memcpy(res_1, resp1, 10);
				memcpy(res_2, resp2, 10);
				memcpy(err_1, error1, 10);
				memcpy(err_2, error2, 10);


				//uart_timeout_start(time_out);
			if(uart_data!=RT_IDEL)
				return uart_data;
			else 
				return RT_HTTP_NOT_FINISH;
			  
//				while (1) {

//						SEGGER_RTT_printf(0, "\n power managment ");
//						if (!is_uart_timer_start) {
//								SEGGER_RTT_printf(0, "\n time out");
//								nrf_delay_ms(10);
//								return RT_TIME_OUT;
//						}

//						if (uart_data != RT_IDEL) {
//								SEGGER_RTT_printf(0, "\nrecieve %s ", recieve);
//								nrf_delay_ms(10);
//								uart_timeout_stop();
//								return uart_data;


//						}
//						power_manage();
//				}
		} else {

				uart_event = false;
				char receive[ BUFFER_SIZE_UP];
				uint8_t time = 0;
				uint8_t response;
				repeat:

				nrf_delay_ms(100);
				time++;
				if (time > time_out) {
						SEGGER_RTT_printf(0, "\n waitForResp_repeat time out");
						nrf_delay_ms(10);
						return RT_TIME_OUT;
				}
				memset(receive, 0, BUFFER_SIZE_UP);
				response = uart_get_string((char*)receive, BUFFER_SIZE_UP);
				SEGGER_RTT_printf(0, "\nrecieve %s ", receive);
				if (strstr(receive, resp1) != NULL || strstr(receive, resp2) != NULL) {
						//SEGGER_RTT_printf(0,"\n waitForResp__ response is  found");
						if (response == RT_FULL)
		goto repeat;
						return RT_SUCCESS;
				} else {
						//SEGGER_RTT_printf(0,"\n the response is  not found");
						if (response == RT_FULL)
		goto repeat;

						if (strstr(receive, error1) != NULL || strstr(receive, error2) != NULL) {
								return RT_ERROR;
						}
	goto repeat;
				}

		}

}


int sendCmdAndWaitForResp(const char*cmd, const char *resp, uint16_t timeout) {
//	SEGGER_RTT_printf(0, "\n sendCmdAndWaitForResp  cmd=%s\tresp=%s .......",cmd,resp);
	nrf_delay_ms(50);	
	uart_new_command = true;
		uart_event = false;
		uart_put_string(cmd);
	//SEGGER_RTT_printf(0, "...  end");
		return waitForResp(resp,timeout); 
		//return wait_uart_event(timeout);

}


int sendCmdAndWaitForResp_handler(const char*cmd, const char *resp, uint16_t timeout) {
//	SEGGER_RTT_printf(0, "\n sendCmdAndWaitForResp  cmd=%s\tresp=%s .......",cmd,resp);
	uart_new_command = true;
				uart_event = true;
				uart_data = RT_IDEL;
				resp_type = RESP_TYPE_1_0;
				memcpy(res_1, resp, 10);
	nrf_delay_ms(50);	
		uart_put_string(cmd);
	//SEGGER_RTT_printf(0, "...  end");
		//return waitForResp(resp,timeout); 
		return wait_uart_event(timeout);

}

int wait_uart_event(uint16_t time_out) {
		//uart_timeout_start(time_out);
		while (1) {
				SEGGER_RTT_printf(0, "\n power managment ");
				if (!is_uart_timer_start) {
						SEGGER_RTT_printf(0, "\n time out");
						nrf_delay_ms(5);
						return RT_TIME_OUT;
				}

				if (uart_data != RT_IDEL) {
						SEGGER_RTT_printf(0, "\nrecieve event %s , data=%s ", recieve,uart_data);
						//uart_timeout_stop();
						nrf_delay_ms(5);
						return uart_data;
				}
				power_manage();
		}
}

void sms_listener(void){
	if (uart_data != RT_IDEL) 
						SEGGER_RTT_printf(0, "\nOOOOOOOO listner  OOOO event %s , data=%s ", recieve,uart_data);
	nrf_delay_ms(5);
}

/**
 * @brief Function for main application entry.
 */
void uart_put_string_len(const char *msg, uint8_t len) {
//SEGGER_RTT_printf(0,"\n put_string:%s ++++++++++++++ ",msg);	
	uart_new_command = true;
		uart_data_found = false;
		for (uint8_t i = 0; i < len; i++) {
				while (app_uart_put(msg[i]) != NRF_SUCCESS) ;
		}
		while (app_uart_put('\r') != NRF_SUCCESS) ;
		while (app_uart_put('\n') != NRF_SUCCESS) ;
}


/**
 * @brief Function for main application entry.
 */
void uart_put_string(const char *msg) {
	//	SEGGER_RTT_printf(0,"\n put_string:%s ++++++++++++++ ",msg);
	 uart_new_command = true;
		uart_data_found = false;
		uint8_t len = strlen((char *)msg);
	
		for (uint8_t i = 0; i < len; i++) {
				while (app_uart_put(msg[i]) != NRF_SUCCESS) ;
		}
		while (app_uart_put('\r') != NRF_SUCCESS) ; //SEGGER_RTT_printf(0,"1-") ;
		while (app_uart_put('\n') != NRF_SUCCESS) ; //SEGGER_RTT_printf(0,"2");

}


int uart_get_string(char *data_array, uint16_t lenght) {
//	SEGGER_RTT_printf(0,"\n get_string:%s ++++++++++++++ ",data_array);	
	uint16_t index = 0;
		while (1) {
				if (app_uart_get((uint8_t *) & data_array[index]) == NRF_ERROR_NOT_FOUND) {
						return RT_SUCCESS;
				}
				nrf_delay_ms(1);
				index++;
				if (index == lenght - 1)
						return RT_FULL;
		}

}
