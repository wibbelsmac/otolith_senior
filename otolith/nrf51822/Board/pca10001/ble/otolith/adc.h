#ifndef ADC_H__
#define ADC_H__

#include "app_error.h"
#include "nrf.h"
#include "nrf_soc.h"
#include "nrf51_bitfields.h"

#define ADC_CAPTURE_COMPARE_0_VALUE 2 // measure at 488hz
#define ADC_TIMER_PRESCALER 1024 // clock ~= 976hz
#define PPI_CHAN1_TO_CONT_READ 1

void adc_config(void);
static void ppi_init(void);

#endif