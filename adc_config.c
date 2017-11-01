
#include "adc_config.h"

#define ADC_BUFFER_SIZE 1
                               /**< Size of buffer for ADC samples.  */
 nrf_adc_value_t       adc_buffer[ADC_BUFFER_SIZE]; /**< ADC buffer. */

 nrf_drv_adc_channel_t m_channel_config= NRF_DRV_ADC_DEFAULT_CHANNEL(NRF_ADC_CONFIG_INPUT_5); /**< Channel instance. Default configuration used. */




/**
 * @brief ADC interrupt handler.
 */
void adc_event_handler(nrf_drv_adc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_ADC_EVT_DONE)
    {
        uint32_t i;
        for (i = 0; i < p_event->data.done.size; i++)
        {
					uint32_t adc= 1200* p_event->data.done.p_buffer[i]*3/1023;
          //  NRF_LOG_INFO("Current sample value: %d\r\n", p_event->data.done.p_buffer[i]);
					SEGGER_RTT_printf(0,"\n ADC hndlr:%d", adc);
        }
    }
}

/**
 * @brief ADC initialization.
 */
 void adc_config(void)
{
    ret_code_t ret_code;
    nrf_drv_adc_config_t config = NRF_DRV_ADC_DEFAULT_CONFIG;

    ret_code = nrf_drv_adc_init(&config, adc_event_handler);
    APP_ERROR_CHECK(ret_code);
m_channel_config.config.config.input=NRF_ADC_CONFIG_SCALING_INPUT_ONE_THIRD;
    nrf_drv_adc_channel_enable(&m_channel_config);
}




unsigned short adc_read(unsigned int adc_in_mask)
{
    uint16_t adc_result;
    // interrupt ADC
    NRF_ADC->INTENSET = (ADC_INTENSET_END_Disabled << ADC_INTENSET_END_Pos); /*!< Interrupt enabled. */
    // config ADC
    NRF_ADC->CONFIG = (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos) /* Bits 17..16 : ADC external reference pin selection. */
                    | (adc_in_mask << ADC_CONFIG_PSEL_Pos)   /*!< Use analog input 0 as analog input. */
                    | (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos) /*!< Use internal 1.2V bandgap voltage as reference for conversion. */
                    | (ADC_CONFIG_INPSEL_AnalogInputOneThirdPrescaling << ADC_CONFIG_INPSEL_Pos) /*!< Analog input specified by PSEL with no prescaling used as input for the conversion. */
                    | (ADC_CONFIG_RES_10bit << ADC_CONFIG_RES_Pos);  /*!< 10bit ADC resolution. */ 
    // enable ADC       
    NRF_ADC->ENABLE = ADC_ENABLE_ENABLE_Enabled; /* Bit 0 : ADC enable. */
    // start ADC conversion
    NRF_ADC->TASKS_START = 1;
    // wait for conversion to end
    while (!NRF_ADC->EVENTS_END)
    {}
    NRF_ADC->EVENTS_END = 0;
    adc_result = NRF_ADC->RESULT*1.2*3*1000/1023;
		//SEGGER_RTT_printf(0,"\r\n lecture adc %d  = %d  \r\n",adc_in_mask,adc_result);  
    //Use the STOP task to save current. Workaround for PAN_028 rev1.1 anomaly 1.
    NRF_ADC->TASKS_STOP = 1;
    return adc_result;
}

