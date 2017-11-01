#ifndef SIM800L_H
#define SIM800L_H

#include "uart_config.h"
void req_post(char * json);
void req_get(void);
/**/
void send_msg(char* msg,char* num);
void check_sim_state(void);
void read_sms (char *nbr);
void set_3g(void);

void POST_request (void);
void GET_request (void);

#endif

