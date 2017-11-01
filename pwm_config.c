#include "pwm_config.h"

APP_PWM_INSTANCE(PWM1,0);                   // Create the instance "PWM1" using TIMER1.

static volatile bool ready_flag;            // A flag indicating PWM status.

void pwm_ready_callback(uint32_t pwm_id)    // PWM callback function
{
    ready_flag = true;
}



void buzzer_on(void){
	nrf_delay_ms(40);
	app_pwm_enable(&PWM1);
  app_pwm_channel_duty_set(&PWM1, 0, 50);   // pwm 50%
	SEGGER_RTT_printf(0,"\nbuzzzzz");

    }

void buzzer_off(void){
	nrf_delay_ms(40);
app_pwm_disable(&PWM1);
SEGGER_RTT_printf(0,"\nbuffff");
}

		
	void pwm_init(uint32_t freq,uint8_t  pin){
ret_code_t err_code;
app_pwm_config_t pwm1_cfg = APP_PWM_DEFAULT_CONFIG_1CH(freq,pin); //pin 23   freq 370
pwm1_cfg.pin_polarity[0] = APP_PWM_POLARITY_ACTIVE_HIGH;
    err_code = app_pwm_init(&PWM1,&pwm1_cfg,pwm_ready_callback);
    APP_ERROR_CHECK(err_code);
    

	}
	



 	