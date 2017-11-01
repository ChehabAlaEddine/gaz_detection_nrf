#include "sim800l.h"
#include "SEGGER_RTT.h"



void req_post(char * json) {
char * req_post="POST /create_clinique_operation HTTP/1.1\r\nHost: compibus.herokuapp.com:80\r\nContent-Length: %d\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n%s";
char  post[250];
char  req[20];
	
	// Starts an TCP connection to the website on port 80
  sendCmdAndWaitForResp("AT+CIPSTART=\"TCP\",\"compibus.herokuapp.com\",80","OK",2000);
	
	sprintf(post,req_post,(strlen(json)),json);
	sprintf(req,"AT+CIPSEND=%d",strlen(post)+2);
 SEGGER_RTT_printf(0,"\n............ %s",post);
	nrf_delay_ms(20);
	while(sendCmdAndWaitForResp(req,">",100)) SEGGER_RTT_printf(0,"\nla requette poste est : %s",req);

 SEGGER_RTT_printf(0,"la requette poste est : %s",post);
	sendCmdAndWaitForResp_handler(post,"200 OK",200);	
}



void req_get(){
SEGGER_RTT_printf(0,"\n req_get");	
	char * get="GET /get_time HTTP/1.1\r\nHost: compibus.herokuapp.com:80\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n";
	char * req;	
	
  sprintf(req,"AT+CIPSEND=%d",strlen(get)+2);
	SEGGER_RTT_printf(0,"\nla requette av get est : %s",req);
 
sendCmdAndWaitForResp("AT+CIPSTART=\"TCP\",\"compibus.herokuapp.com\",80","OK",200);
nrf_delay_ms(300);
	
	
	while(sendCmdAndWaitForResp(req,">",2000));
	while(sendCmdAndWaitForResp_handler(get,"SEND OK",4000));
	//sendCmdAndWaitForResp_handler(get,"}",2000);
SEGGER_RTT_printf(0,"\n***********************************************************");

	}

	
	void set_3g(void)
	{ 
		SEGGER_RTT_printf(0,"\nConnection setting-----");
		 sendCmdAndWaitForResp("AT+CFUN=1","OK",200);// enables full functionality of the modem
	while( sendCmdAndWaitForResp("AT+CSTT=\"internet.ooredoo.tn\",\"\",\"\"","OK",1000)) nrf_delay_ms(500); 

		while(true){
		if(sendCmdAndWaitForResp("AT+CSQ","+CSQ: 1",200)==RT_SUCCESS) break;
			else {if(sendCmdAndWaitForResp("AT+CSQ","+CSQ: 2",200)==RT_SUCCESS) break;}
		}
	
		while(	sendCmdAndWaitForResp("AT+CIICR","OK",500));
nrf_delay_ms(100);  // SEGGER_RTT_printf(0,"\npass1***********************************************************");
sendCmdAndWaitForResp("AT+CIFSR","OK",200); //ip adress
	}

void send_msg(char* msg,char* num){
//	SEGGER_RTT_printf(0,"\n try to send SMS ..... ");
//	sendCmdAndWaitForResp("AT+CMGF=1","OK",200);
//	nrf_delay_ms(1000);
//	sendCmdAndWaitForResp("AT+CMGW=\"20232860\"<CR>Sending text messages is easy<Ctrl+z>","OK",200); nrf_delay_ms(1000);
//	char buff[100];
//	sprintf(buff,"AT+CMGS=\"20232860\"\r\nSending text messages is easy%c",(char)26);
//	nrf_delay_ms(1000);
//	SEGGER_RTT_printf(0,"send sms %s ", buff);
//	sendCmdAndWaitForResp(buff,"OK",200);
//	//sendCmdAndWaitForResp("AT+QCCID","<",200);
//	SEGGER_RTT_printf(0,"\n end of sending SMS !! ");
//	
	
	char cmd[100];
	sprintf(cmd,"AT+CMGS=\"%s\"",num);
   sendCmdAndWaitForResp(cmd,">",200);
	 nrf_delay_ms(200);
	 sprintf(cmd,"%s %c",msg,(char) 26);
   SEGGER_RTT_printf(0,"\n the message is %s \n",cmd);
   sendCmdAndWaitForResp(cmd,"OK",200);
}

void check_sim_state(void){
	sendCmdAndWaitForResp("AT","OK",1000); nrf_delay_ms(200);
while(sendCmdAndWaitForResp("AT+CPIN?","OK",1000))
	 SEGGER_RTT_printf(0," \t  cheking sim state !! ");
	nrf_delay_ms(100);
	sendCmdAndWaitForResp("AT+csq","+",200);
}


void read_sms (char *nbr) {
  sendCmdAndWaitForResp("AT+CMGF=1","OK",500);
	char cmd[20] ="AT+CMGR=";
	sprintf(cmd,"AT+CMGR=%s",nbr);
	SEGGER_RTT_printf(0," \n reading with commande:%s",cmd);
  while(sendCmdAndWaitForResp(cmd,"OK",2000))SEGGER_RTT_printf(0," \n reading with commande:%s",cmd);;
 

}





//////////////////////////ines
void GET_request (void){

	// Starts an TCP connection to the website on port 80
	sendCmdAndWaitForResp("AT+CIPSTART=\"TCP\",\"compibus.herokuapp.com\",80","OK",200);
	nrf_delay_ms(200);
	// send request
	char * get= "GET /get_time HTTP/1.1\r\nHost: compibus.herokuapp.com:80\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n";
	char  cmd[20] ="AT+CIPSEND=";
	sprintf(cmd,"AT+CIPSEND=%d",strlen(get)+2);
	int result2=5; // 5: RT_ERROR , 0: RT_SUCCESS
	while (result2!=0){
		result2 =sendCmdAndWaitForResp(cmd,">",2000);
		SEGGER_RTT_printf(0,"result2: %d",result2);
	}
	sendCmdAndWaitForResp_handler(get,"SEND OK",4000);	
}

void POST_request (void){

	// Starts an TCP connection to the website on port 80
	sendCmdAndWaitForResp("AT+CIPSTART=\"TCP\",\"compibus.herokuapp.com\",80","OK",200);
	// send request
	char * json = "{\"nurse_id\":255,\"bed_id\":255,\"bed_voltage\":255,\"action\":\"test\"}"; 
	char  post [250];
  sprintf(post,"POST /create_clinique_operation HTTP/1.1\r\nHost: compibus.herokuapp.com:80\r\nContent-Length:%d\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n%s",strlen(json),json);
	//	SEGGER_RTT_printf(0,"\n post!!! : %s \n",post);
	char  cmd[20] ="AT+CIPSEND=";
	sprintf(cmd,"AT+CIPSEND=%d",strlen(post)+2);
	int result2=5; // 5: RT_ERROR , 0: RT_SUCCESS
	while (result2!=0){
		result2 =sendCmdAndWaitForResp(cmd,">",2000);
		SEGGER_RTT_printf(0,"result2: %d",result2);
	}
	sendCmdAndWaitForResp_handler(post,"SEND OK",1000);	
}
