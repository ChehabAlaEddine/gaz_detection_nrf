/* Copyright (c) 2014 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 *
 */

/** @file
 *
 * @defgroup ble_sdk_uart_over_ble_main main.c
 * @{
 * @ingroup  ble_sdk_app_nus_eval
 * @brief    UART over BLE application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service.
 * This application uses the @ref srvlib_conn_params module.
 */

#include <stdint.h>
#include <string.h>
#include "pwm_config.h"
#include "app_gpiote.h"
#include "app_button.h"
#include "SEGGER_RTT.h"
#include "nrf_delay.h"
#include "module.h"
#include "ble_config.h"
#include "adc_config.h"
#include "uart_config.h"
#include "sim800l.h"
#include "fstorage.h"
#include "module.h"

#define  BUTTON_DEBOUNCE_DELAY			 APP_TIMER_TICKS(5, APP_TIMER_PRESCALER) // Delay from a GPIOTE event until a button is reported as pushed. 
#define  Button       15
#define  BUZZER_PIN   16
#define  ADC_PIN1     NRF_ADC_CONFIG_INPUT_3    //pin2

#define  ADC_PIN2     NRF_ADC_CONFIG_INPUT_4  //6  //pin5


                 /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */
#define  ADC_BUFFER_SIZE 1 


//operation Cmd
#define OPERATION_NULL        0
#define OPERATION_TEST_PW     1
#define OPERATION_CHANGE_PW   2
#define OPERATION_BUZZER_OFF  3
#define OPERATION_CHANGE_NUM  4
#define OPERATION_CHANGE_MSG  5
#define OPERATION_CHANGE_NAME 6
#define OPERATION_SEND_LIST   7
#define OPERATION_DEL_NUM     8

#define NUM_PAGES 4



#define SEC_PARAM_BOND                   1                                          /**< Perform bonding. */
#define SEC_PARAM_MITM                   0                                          /**< Man In The Middle protection not required. */
#define SEC_PARAM_LESC                   0                                          /**< LE Secure Connections not enabled. */
#define SEC_PARAM_KEYPRESS               0                                          /**< Keypress notifications not enabled. */
#define SEC_PARAM_IO_CAPABILITIES        BLE_GAP_IO_CAPS_NONE                       /**< No I/O capabilities. */
#define SEC_PARAM_OOB                    0                                          /**< Out Of Band data not available. */
#define SEC_PARAM_MIN_KEY_SIZE           7                                          /**< Minimum encryption key size. */
#define SEC_PARAM_MAX_KEY_SIZE           16         


extern  bool myBle_connection_state;
extern  uint16_t   m_conn_handle;
extern  nrf_adc_value_t adc_buffer[ADC_BUFFER_SIZE];
extern  bool ADV_state, lock ;
extern  nrf_drv_adc_channel_t m_channel_config;
uint8_t operation=0;

gaz_module  gaz_module_;
State state=normal;

//bool alert=false;
//char device_name[20]="Chehab_BLE2", 
//	   device_pw[20]="ala1",
//     num[10]="20232860",
//     msg[20]="prem msg",
char   MSG_STOP[16]="STOP ALERT",
     buffT[11];
char * s="";char*last; //for sms : nbr of sms

uint8_t c=5;
const uint32_t * stored_data;   //for  fstorage
char  arg[20];
char * cmd_key;
char ** cmd_arry= NULL;
char ** nbr_arry= NULL;
char* recv;
uint8_t cc; //compteur bazzer
static uint8_t fs_callback_flag;
static void fstorage_update();

//////****** fstorage ******/////////
const uint32_t * stored_data;

 void fstorage_update2(void);

static void fs_evt_handler(fs_evt_t const * const evt, fs_ret_t result)
{
	SEGGER_RTT_printf(0,"fs_evt_handler ............d\n\r");  
    if (result != FS_SUCCESS)
    { fs_callback_flag=1;
        bsp_indication_set(BSP_INDICATE_FATAL_ERROR);
    SEGGER_RTT_printf(0,"fstorage handler faild\n\r");    
		}
  else
  {
    SEGGER_RTT_printf(0,"fstorage handler success\n\r");    
    fs_callback_flag = 0;
  }
}


FS_REGISTER_CFG(fs_config_t fs_config) =
    {
        .callback  = fs_evt_handler,    // Function for event callbacks.
        .num_pages = NUM_PAGES ,  //1,       //NUM_PAGES=4,          // Number of physical flash pages required.
        .priority  = 0xFE               // Priority for flash usage.
    };

		
		////////end fstorage
		
void exe_fond(){
	//SEGGER_RTT_printf(0,"\n arr0:%s \t arr1:%s \t arg:%s \t cmd_key:%s",cmd_arry[0],cmd_arry[1],arg,cmd_key);
uint8_t count=0;
	switch(operation)	{
		
		case OPERATION_DEL_NUM:
		SEGGER_RTT_printf(0,"\nin DEL_NUM with arg: %d",atoi(arg));
	  uint8_t num=atoi(arg);
		char * aux="null";
		memset(nbr_arry[num],0,8);
		strcpy(nbr_arry[num],aux);	
    SEGGER_RTT_printf(0,"\nbefore update:num %s has ben deleted",arg);	
		fstorage_update2();
		SEGGER_RTT_printf(0,"\nnum %s has ben deleted",arg);
		break;
		
		case OPERATION_SEND_LIST :
		SEGGER_RTT_printf(0,"\nin operation sending nums");
		
		while(count<10) {
		sprintf(buffT,"NUM;%d;%s;",count,nbr_arry[count]); 
		sending_msg(buffT);
		SEGGER_RTT_printf(0,"\nsend: %s",buffT);
		count++;
		nrf_delay_ms(400);}
		
		sending_msg("END_SEND_LIST;");
		SEGGER_RTT_printf(0,"\noperation sending numsend");
		break;
		
		case OPERATION_TEST_PW :  //verifier modpasse
			if(!strcmp(gaz_module_.device_pw,arg)){ //mot de pass valide
	  		sending_msg("PW_VALIDE;");
		  	SEGGER_RTT_printf(0,"\n moodpasse %s est valide",arg);
			   }
			else {sending_msg("PW_INVALIDE;"); //mot de pass invalide
				SEGGER_RTT_printf(0,"\n moodpasse %s est invalide---mp:%s",arg,gaz_module_.device_pw);
	       }
   break;
		

		case OPERATION_CHANGE_PW : //changer modpasse
		  
			  memset(gaz_module_.device_pw,0,20);
				strcpy(gaz_module_.device_pw,arg);
	      fstorage_update();
				SEGGER_RTT_printf(0,"\n PW est chnangé a %s ",gaz_module_.device_pw);
			  sending_msg("Done");
			
		break;

		
    case OPERATION_CHANGE_NUM : //changer numero
					SEGGER_RTT_printf(0,"\nin add num %d ",arg);
				//		while(count<4){
				//		memset(nbr_arry[count],0,10);
				//		strcpy(nbr_arry[count++],"11112222");
				//		}	
				count=0;
						
				while(count<10){
				SEGGER_RTT_printf(0,"\ncount %d val %s",count,nbr_arry[count]);
				//((!strcmp(nbr_arry[count],"00000000"))
					if((!atoi(nbr_arry[count]))||((strlen(nbr_arry[count]))!=8)) break;
					else count++;
				}		
          SEGGER_RTT_printf(0,"\ncount %d ",count);
				if(count>=10){nrf_delay_ms(500); sending_msg("NMBER_FULL");break;}
				else{	memset(nbr_arry[count],0,8);
						strcpy(nbr_arry[count],arg);
								 fstorage_update2();
								 SEGGER_RTT_printf(0,"\n un num est ajouté %s a la pos %d",nbr_arry[count],count);
								 sending_msg("Done");
					      }
			break;
				
		case OPERATION_CHANGE_MSG : //changer msg
			  memset(gaz_module_.msg,0,20);
				strcpy(gaz_module_.msg,arg);
	      fstorage_update();
		    SEGGER_RTT_printf(0,"\n msg est chnangé a %s ",gaz_module_.msg);
			  sending_msg("Done");
break;
		
  	case OPERATION_BUZZER_OFF : 
		  SEGGER_RTT_printf(0,"\n ----mobile cmd--Buzzer Off-----------");
	gaz_module_.state=normal;
		sending_msg("BUZZ_STOPED;"); //retour de confirmation 
		
		//SEGGER_RTT_printf(0,"\ndevice_name: %s \t device_pw: %s \t num: %s \t msg: %s  ",device_name,device_pw,num,msg);
    //SEGGER_RTT_printf(0,"\n------l'alert est arreté par mobile --8888888888888888");	 
		//nrf_delay_ms(500);
		break;
		
		case OPERATION_CHANGE_NAME :
			  memset(gaz_module_.device_name,0,20);
				strcpy(gaz_module_.device_name,arg);
		   fstorage_update();  
		  sending_msg("Done");
	//	NVIC_SystemReset();  //reboot
		
		lock=true;  ADV_state=true; //turn off ble
     on_off_advertising();
		nrf_delay_ms(2000);
    gap_params_init(arg);
    nrf_delay_ms(200);		//reparametrage
    lock=true;  ADV_state=false;  //turn on  ble
    on_off_advertising();
	  SEGGER_RTT_printf(0,"BLE nom est changé a %s",gaz_module_.device_name);
     break;
	}
	
	operation=OPERATION_NULL;	
	c=0;
}



void exe_cmd(char *cmd){

	cmd_arry=str_split(cmd,";");
	 cmd_key=cmd_arry[0];
	      strcpy(arg,cmd_arry[1]);       
	SEGGER_RTT_printf(0,"\n reception: %s  \t\t cmd_kay: %s \t arg: %s ",cmd,cmd_key,arg);	
	
	if(!strcmp(cmd_key,"PW"))			         operation=OPERATION_TEST_PW;
	else if (!strcmp(cmd_key,"CPW"))       operation=OPERATION_CHANGE_PW;
	else if (!strcmp(cmd_key,"BOFF"))      operation=OPERATION_BUZZER_OFF;
	else if (!strcmp(cmd_key,"CNUM"))      operation=OPERATION_CHANGE_NUM;
  else if (!strcmp(cmd_key,"CMSG"))      operation=OPERATION_CHANGE_MSG;	
	else if (!strcmp(cmd_key,"CNAME"))     operation=OPERATION_CHANGE_NAME;
	else if (!strcmp(cmd_key,"SEND_LIST")) operation=OPERATION_SEND_LIST;
	else if (!strcmp(cmd_key,"DEL_NUM"))   operation=OPERATION_DEL_NUM;
	else {                                 operation=OPERATION_NULL;
 
		sending_msg("Commande invalide");
		SEGGER_RTT_printf(0,"\n Commande invalide");
	}
	
	if(operation!=OPERATION_NULL) gaz_module_.state=config;	
	//SEGGER_RTT_printf(0," \n commande : %s \t operation : %d",cmd_key,operation);
	c=0;
	//exe_fond();
  }

	
	

	//srt cut  SUB STRING
	char *str_sub (const char *s, unsigned int start, unsigned int end)
{
   char *new_s = NULL;

   if (s != NULL && start < end)
   {
/* (1)*/
      new_s = malloc (sizeof (*new_s) * (end - start + 2));
      if (new_s != NULL)
      {
         int i;

/* (2) */
         for (i = start; i <= end; i++)
         {
/* (3) */
            new_s[i-start] = s[i];
         }
         new_s[i-start] = '\0';
      }
      else
      {
         fprintf (stderr, "Memoire insuffisante\n");
         return NULL;
      }
   }
   return new_s;
}



//Str Split Fnct

char **str_split (char *s, const char *ct)
{
   char **tab = NULL;

   if (s != NULL && ct != NULL)
   {
      int i;
      char *cs = NULL;
      size_t size = 1;

/* (1) */
      for (i = 0; (cs = strtok (s, ct)); i++)
      {
         if (size <= i + 1)
         {
            void *tmp = NULL;

/* (2) */
            size <<= 1;
            tmp = realloc (tab, sizeof (*tab) * size);
            if (tmp != NULL)
            {
               tab = tmp;
            }
            else
            {
               fprintf (stderr, "Memoire insuffisante\n");
               free (tab);
               tab = NULL;
               return NULL;
            }
         }
/* (3) */
         tab[i] = cs;
         s = NULL;
      }
      tab[i] = NULL;
   }
   return tab;
}




void read_adc(){
APP_ERROR_CHECK(nrf_drv_adc_buffer_convert(adc_buffer,ADC_BUFFER_SIZE));
        uint32_t i;
        for (i = 0; i < ADC_BUFFER_SIZE; i++)
        {// manually trigger ADC conversion
            nrf_drv_adc_sample();
           
				}
}



//Timer 1 handler
void timer_handler(void * p_context)
{ 

}


void timer1_handler(void * p_context)
{
	//sendCmdAndWaitForResp("AT","OK",100);
	if(gaz_module_.state==alert){
//		if(cc%3)buzzer_on();    //buzzer
//       else buzzer_off();
    SEGGER_RTT_printf(0,"\n Alert!!!!!!!!!%s!!!!!!!!!!!Buzzer",cc++);
		if(myBle_connection_state)sending_msg("ALERT;");
	}
}

static void button_handler(uint8_t pin_no, uint8_t button_action)
{
    if(button_action == APP_BUTTON_PUSH)
    {
        switch(pin_no)
					{
					case Button :
					{SEGGER_RTT_printf(0,"\n Button is pressed ");
           lock=true; 
						if(ADV_state) ADV_state=false;
						else ADV_state=true;
					}

						break;	
				}
	}
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */

void sleep_mode_enter(void)
{
    uint32_t err_code;
    // Prepare wakeup buttons.
    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}




/**@brief Function for initializing buttons and leds.
 *
 * @param[out] p_erase_bonds  Will be true if the clear bonding button was pressed to wake the application up.
 */
static void buttons_leds_init(bool * p_erase_bonds)
{	
	
	
	static app_button_cfg_t p_button[] = {{Button, APP_BUTTON_ACTIVE_HIGH, NRF_GPIO_PIN_PULLDOWN, button_handler}  };
	// Macro for initializing the GPIOTE module.
  //It will handle dimensioning and allocation of the memory buffer required by the module, making sure that the buffer is correctly aligned.
    APP_GPIOTE_INIT(1);
	//Initializing the buttons.
    uint32_t err_code = app_button_init(p_button, 1, BUTTON_DEBOUNCE_DELAY);
    APP_ERROR_CHECK(err_code);
	 // Enabling the buttons.										
    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
																				 
																				 
//    bsp_event_t startup_event;
//    uint32_t err_code = bsp_init(BSP_INIT_LED | BSP_INIT_BUTTONS,
//                                 APP_TIMER_TICKS(100, APP_TIMER_PRESCALER),
//                                 bsp_event_handler);
//    APP_ERROR_CHECK(err_code);

//    err_code = bsp_btn_ble_init(NULL, &startup_event);
//    APP_ERROR_CHECK(err_code);

//    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


/**@brief Function for placing the application in low power state while waiting for events.
 */
void power_manage(void)
{
    uint32_t err_code = sd_app_evt_wait();
    APP_ERROR_CHECK(err_code);
}

////////starage example ////////////////
static void fstorage_init(){
	fs_ret_t retr = fs_init();
  if (retr != FS_SUCCESS) SEGGER_RTT_printf(0,"FS ERROR OCCURED\n");
	else SEGGER_RTT_printf(0,"init successs \n");
}
	
			 void fstorage_readi(uint8_t i){
//readdata
	SEGGER_RTT_printf(0,"\nReading from flash address 0x%X+%d:   \t", (uint32_t)fs_config.p_start_addr,i);
				uint32_t flash_data2 = *(fs_config.p_start_addr+i);
				SEGGER_RTT_printf(0,"%X --> %s", flash_data2,(char )flash_data2);
	nrf_delay_ms(100);
}

			 void fstorage_read(){
fstorage_readi(0);
}


static void fstorage_extract(){
	SEGGER_RTT_printf(0,"\nextracting1");	
	char  String[80];
	memcpy(String,fs_config.p_start_addr,80);
		     
		cmd_arry=str_split(String,";");
		strcpy(gaz_module_.device_name,cmd_arry[0]);
		strcpy(gaz_module_.device_pw,cmd_arry[1]);
		strcpy(gaz_module_.num,cmd_arry[2]);
		strcpy(gaz_module_.msg,cmd_arry[3]);	
SEGGER_RTT_printf(0,"\n cmd_arry:%s \n after extraction: devicename:%s;devPW:%s;num:%s;msg:%s;", String,gaz_module_.device_name, gaz_module_.device_pw, gaz_module_.num, gaz_module_.msg); 
	SEGGER_RTT_printf(0,"\nextracting1: 0x%X\t0x%X\n",fs_config.p_start_addr,fs_config.p_start_addr+1000);
	//char  buff[80];
//	sprintf(buff,"\n cmd_arry:%s \n after extraction: devicename:%s;devPW:%s;num:%s;msg:%s;", String,gaz_module_.device_name, gaz_module_.device_pw, gaz_module_.num, gaz_module_.msg); 
	//SEGGER_RTT_printf(0,"\n buff=%s",buff);
}

static void fstorage_extract2(){
	SEGGER_RTT_printf(0,"\nextracting22222");	
	char  String[100];
	memcpy(String,fs_config.p_start_addr+256,100);
		     
		nbr_arry=str_split(String,";");
//		strcpy(gaz_module_.device_name,cmd_arry[0]);
//		strcpy(gaz_module_.device_pw,cmd_arry[1]);
//		strcpy(gaz_module_.num,cmd_arry[2]);
//		strcpy(gaz_module_.msg,cmd_arry[3]);	
//SEGGER_RTT_printf(0,"\n cmd_arry:%s \n after extraction: devicename:%s;devPW:%s;num:%s;msg:%s;", String,gaz_module_.device_name, gaz_module_.device_pw, gaz_module_.num, gaz_module_.msg); 

	//char  buff[80];
//	sprintf(buff,"\n cmd_arry:%s \n after extraction: devicename:%s;devPW:%s;num:%s;msg:%s;", String,gaz_module_.device_name, gaz_module_.device_pw, gaz_module_.num, gaz_module_.msg); 
//	SEGGER_RTT_printf(0,"\n buff=%s",String);
//	SEGGER_RTT_printf(0,"\n arr=%s",nbr_arry[0]);
//	SEGGER_RTT_printf(0,"\n arr=%s",nbr_arry[4]);
//	SEGGER_RTT_printf(0,"\n arr=%s",nbr_arry[8]);
//	SEGGER_RTT_printf(0,"\n arr=%s",nbr_arry[15]);
//	nrf_delay_ms(12000);
}

static void fstorage_update(){
  //make the storage string contain device_name; device_pw; num; msg
	char  buff[80];
	//uint32_ newPage;
	sprintf(buff,"%s;%s;%s;%s;", gaz_module_.device_name, gaz_module_.device_pw, gaz_module_.num, gaz_module_.msg); 
 // sprintf(newPage,"%X",fs_config.p_start_addr+1000); 
	char * aux2=buff;
	uint16_t num=(strlen(buff)/4)+1;
	
	//SEGGER_RTT_printf(0,"\n buff=%s\n aux2=%s  \tlenth=%d \n",buff,aux2,strlen(aux2));
	
	//erasing flash
	fs_callback_flag = 1;
	fs_ret_t ret = fs_erase(&fs_config,fs_config.p_start_addr,1,fs_evt_handler);
	if (ret != FS_SUCCESS) SEGGER_RTT_printf(0,"FS ERROR OCCURED erase page 0   \t ERR:%d \n",ret);
	   else SEGGER_RTT_printf(0,"erase page 0 successs \n");
	while(fs_callback_flag == 1)  { power_manage(); }
	
	//writing data
		fs_callback_flag = 1;
		ret = fs_store(&fs_config, fs_config.p_start_addr,(uint32_t *)((char*)buff),num, fs_evt_handler);      //Write data to memory address 0x0003F000. Check it with command: nrfjprog --memrd 0x0003F000 --n 16
		if (ret != FS_SUCCESS) SEGGER_RTT_printf(0,"FS ERROR OCCURED write first byte\n");
	   else SEGGER_RTT_printf(0,"write successs \n");
	  while(fs_callback_flag == 1)  { power_manage();
			}SEGGER_RTT_printf(0,"end update........... \n");
}

 void fstorage_update2(void){
  //make the storage string contain device_name; device_pw; num; msg
	SEGGER_RTT_printf(0,"in update 2..........");
	 char  buff[120];
	uint8_t cunt=0;
  while(cunt++<10) 
		if((strlen(nbr_arry[cunt-1]))!=8) { 
			memset(nbr_arry[cunt-1],0,8);
		strcpy(nbr_arry[cunt-1],"__null__");
     nrf_delay_ms(200);		}
	 
sprintf(buff,"%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;",nbr_arry[0],nbr_arry[1],
	 nbr_arry[2],nbr_arry[3],nbr_arry[4],nbr_arry[5]
	 ,nbr_arry[6],nbr_arry[7],nbr_arry[8],nbr_arry[9]);

	 //sprintf(buff,"%s;",nbr_arry[cunt++]); 
	//SEGGER_RTT_printf(0,"in update 2...........\nbuff=%s ",buff);
 // sprintf(newPage,"%X",fs_config.p_start_addr+1000); 
	//char * aux2=buff;
	uint16_t num=(strlen(buff)/4)+1;
	
	//SEGGER_RTT_printf(0,"\n buff=%s\n aux2=%s  \tlenth=%d \n",buff,aux2,strlen(aux2));
	
	//erasing flash
	fs_callback_flag = 1;
	fs_ret_t ret = fs_erase(&fs_config,fs_config.p_start_addr+256,1,fs_evt_handler);
	if (ret != FS_SUCCESS) SEGGER_RTT_printf(0,"FS ERROR OCCURED erase page 1   \t ERR:%d \n",ret);
	   else SEGGER_RTT_printf(0,"erase page 1 successs \n");
	while(fs_callback_flag == 1)  { power_manage(); }
	
	//writing data
		fs_callback_flag = 1;
		ret = fs_store(&fs_config, fs_config.p_start_addr+256,(uint32_t *)((char*)buff),num, fs_evt_handler);      //Write data to memory address 0x0003F000. Check it with command: nrfjprog --memrd 0x0003F000 --n 16
		if (ret != FS_SUCCESS) SEGGER_RTT_printf(0,"FS ERROR OCCURED write first byte\n");
	else SEGGER_RTT_printf(0,"write successs at 0x%X\n qte:%d",fs_config.p_start_addr+256,num);
	  while(fs_callback_flag == 1)  { power_manage();
			}SEGGER_RTT_printf(0,"end update2........... \n");
}	

void routine ( void ){
	
		if(gaz_module_.state==normal){			//en cas normal "pas d'alerte"
					uint16_t		 val2=adc_read(ADC_PIN2)/5; 
		//	sprintf(buffT,"gaz_val2;%d;",adc_read(ADC_PIN1));   // Ao    500 sans gaz  -- 1200 gaz
	//		SEGGER_RTT_printf(0,"\t val2: %d ",val);          //Do H/L      3500 sans gaz - 850 gaz
		//	sending_msg(buffT); 
		
		uint16_t	val=adc_read(ADC_PIN1);
			sprintf(buffT,"gaz_val;%d;%d;",val,val2);   // Ao    500 sans gaz  -- 1200 gaz
			SEGGER_RTT_printf(0,"\n val1: %d \tval2:%d",val,val2);
			sending_msg(buffT);
	
					if((val>750)||(val2>6666)){
					gaz_module_.state=alert;       //detection grand teneur en gaz
					SEGGER_RTT_printf(0,"\n l'alert est declanchee");
					//req_post();	//requette
				//	send_msg(gaz_module_.msg,gaz_module_.num);	//send msg
						cc=0;
					
						//initialisation de nbr de msg pour la detection larrivee de nouveau msg 
						while(sendCmdAndWaitForResp("AT+CPMS=\"SM\"","+CPMS:",500))if(gaz_module_.state!=alert) break;
					s=(str_split(str_sub(recv,22,40),","))[0];
					last=s;
						}
					
				}
		
		else if(gaz_module_.state==alert) {        //en cas d alerte
				SEGGER_RTT_printf(0,"\nalert state");
			 //verifecation de reception d un mesage de controle
				if(strstr(last,s)){ 			//waiting for sms
					last=s;	
					while(sendCmdAndWaitForResp("AT+CPMS=\"SM\"","+CPMS:",500)) if(gaz_module_.state!=alert) break;
					s=(str_split(str_sub(recv,22,40),","))[0];
				SEGGER_RTT_printf(0,"\nverifier l arrive d un msg !!!!!!!!!!!");
				} 

					else{     //arrive de nv sms
					char	aux[3];
						sprintf(aux,"%s",s);
						read_sms(aux);
						s=(str_split(str_sub(recv,27,150),","))[1];
						SEGGER_RTT_printf(0,"\nun nv msg arrivé est :%s",s);
							if(!strcmp(s,MSG_STOP)) {
								 gaz_module_.state=normal;
							//	sending_msg("BUZZ_STOPED");
									SEGGER_RTT_printf(0,"\n l'alert est arreté avec un msg");
							
								 }
					}
			}
		
			else if(gaz_module_.state==config){
				   exe_fond();
				    gaz_module_.state=normal;
				//nrf_delay_ms(2000);
			}
			
			c=4;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(void)
{   
SEGGER_RTT_printf(0,"\n debut programme **************************");
gaz_module_init(&gaz_module_);
	
	//ble
    uint32_t err_code;
    bool erase_bonds;
	  fs_ret_t ret;
    //adc
	  // Initialize.
    APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
	
    adc_config();
   

  // Initialize.
    //APP_TIMER_INIT(APP_TIMER_PRESCALER, APP_TIMER_OP_QUEUE_SIZE, false);
   
   	
    buttons_leds_init(&erase_bonds);
    ble_stack_init();
	
	fstorage_init();
   fstorage_extract(); //en cas de donne ecrite en flash si nn commentter le extracte() et decomnter le update()  et decommenter linialisation dans gaz_module.init  
	// fstorage_update2();
	 fstorage_extract2(); 
//	fstorage_update();
    gap_params_init(gaz_module_.device_name);
    services_init();
    advertising_init();
    conn_params_init();
		pwm_init((uint16_t)370,BUZZER_PIN);  //buzzer init 370:tonalité
	
   
	 nrf_delay_ms(500);

//uart
  nrf_gpio_cfg_output(4); //pin 29 en MQ7  est pour lactivation
	nrf_gpio_pin_set(4);
	nrf_gpio_cfg_output(29); //pin 29 en sim900  est pour lactivation
	nrf_gpio_pin_set(29);
	nrf_delay_ms(500);
  uart_init();

SEGGER_RTT_printf(0,"\n try to connect");
    err_code = ble_advertising_start(BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
		lock=true;  ADV_state=false;   // lock=true :donne de permission pour une seule entree ala fonction de changement d etat ADV_state 
 on_off_advertising();
	 nrf_delay_ms(500);


//timer
//	   APP_TIMER_DEF(m_timer_id);    //T1
//	    err_code = app_timer_create(&m_timer_id, APP_TIMER_MODE_REPEATED, timer_handler);
//      err_code = app_timer_start(m_timer_id, APP_TIMER_TICKS(500, APP_TIMER_PRESCALER), NULL);
	    APP_TIMER_DEF(m_timer1_id); //T2
      err_code = app_timer_create(&m_timer1_id, APP_TIMER_MODE_REPEATED, timer1_handler);
      err_code = app_timer_start(m_timer1_id, APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER), NULL);



//sendCmdAndWaitForResp("AT+CNMI=1,2,0,0,0","OK",500); //demande du sim900 une notification lors de larrivee d un msg
//req_post("{\"nurse_id\":255,\"bed_id\":255,\"bed_voltage\":255,\"action\":\"test\"}");

SEGGER_RTT_printf(0,"\n...........LOOOP .................................  ");
int i=0;

    for(;;)
    {routine();	
			SEGGER_RTT_printf(0,"\n cycl %d ...............................................%d ",i++,gaz_module_.state);
				nrf_delay_ms(c*200);  // pour une delay variable-- longue en cas normal et courte en cas de exucution d une tache comme les commandes du mobile
	    c=3;
        power_manage();

		}
}
