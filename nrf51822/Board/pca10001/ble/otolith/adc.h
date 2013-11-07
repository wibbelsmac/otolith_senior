#ifndef ADC_H__
#define ADC_H__

#include "app_util.h"

#define ADC_CAPTURE_COMPARE_0_VALUE 0x6F
#define ADC_TIMER_PRESCALER 0xF
#define PPI_CHAN1_TO_CONT_READ 1
#define ADC_IRQ_PRI APP_IRQ_PRIORITY_HIGH
#define ADC_IN_PIN_NUMBER (1)
void adc_config(void);
static void ppi_init(void);
static void timer2_init(void);
void set_adc_pin_select(uint8_t adc_in);
#endif
