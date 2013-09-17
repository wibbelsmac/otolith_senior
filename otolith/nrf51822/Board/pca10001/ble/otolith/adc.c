#include "adc.h"
#include "app_error.h"
#include "nrf.h"
#include "nrf51_bitfields.h"
#include "util.h"

void ADC_IRQHandler(void) {
	NVIC_DisableIRQ(ADC_IRQn);
	NVIC_ClearPendingIRQ(ADC_IRQn);
  
  mlog_str("ADC Handler");
	if(NRF_ADC->BUSY) {
    return;
  }

  mlog_println("ADC: ", NRF_ADC->RESULT);
	NVIC_EnableIRQ(ADC_IRQn);
}

void adc_config(void) {
	NVIC_DisableIRQ(ADC_IRQn);
  // ADC must be off to configure
  if(NRF_ADC->BUSY) {
		mlog_str("ADC Busy");
    NRF_ADC->TASKS_STOP;
  }

  NRF_ADC->CONFIG = (
    (ADC_CONFIG_PSEL_AnalogInput0 << ADC_CONFIG_PSEL_Pos)|
    (ADC_CONFIG_RES_8bit << ADC_CONFIG_RES_Pos)|
    (ADC_CONFIG_INPSEL_AnalogInputNoPrescaling << ADC_CONFIG_INPSEL_Pos)|
    (ADC_CONFIG_REFSEL_VBG << ADC_CONFIG_REFSEL_Pos)|
    (ADC_CONFIG_EXTREFSEL_None << ADC_CONFIG_EXTREFSEL_Pos)
  );

  ppi_init();
  timer2_init();

  NRF_ADC->ENABLE = 1;
  NVIC_EnableIRQ(ADC_IRQn);
}

static void ppi_init(void) {

	NRF_PPI->CH[3].EEP = (uint32_t)&(NRF_TIMER2->EVENTS_COMPARE[0]);
	NRF_PPI->CH[3].TEP = (uint32_t)&(NRF_ADC->TASKS_START);
	NRF_PPI->CHENCLR = PPI_CHEN_CH3_Msk;
	NRF_PPI->CHENSET = PPI_CHEN_CH3_Msk;
}

static void timer2_init(void)
{
    // Configure timer
    NRF_TIMER2->MODE      = TIMER_MODE_MODE_Timer;
    NRF_TIMER2->BITMODE   = TIMER_BITMODE_BITMODE_08Bit;
    NRF_TIMER2->PRESCALER = ADC_TIMER_PRESCALER;

    // Clear the timer
    NRF_TIMER2->TASKS_CLEAR = 1;

    // Load the value to TIMER2 CC0 register. 
    NRF_TIMER2->CC[0] = ADC_CAPTURE_COMPARE_0_VALUE;

    // Make the Capture Compare 0 event to clear the timer. This will restart the timer.
    NRF_TIMER2->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos;

    // There is no need to setup NRF_TIMER2->INTENSET register because the application do not need
    // to wake up the CPU on Timer interrupts.
}


