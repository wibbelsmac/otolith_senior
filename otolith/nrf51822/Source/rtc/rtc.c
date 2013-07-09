/*
 * Real Time Counter
 */ 

#include "nrf.h"
#include <rtc.h>

void lfclk_config(void) {
  NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSRC_SRC_Xtal << CLOCK_LFCLKSRC_SRC_Pos);
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_LFCLKSTART = 1;
  while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
  {
  }
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
}

void rtc_config(void) {
  NRF_RTC0->PRESCALER = COUNTER_PRESCALER;   // Set prescaler to a TICK of RTC_FREQUENCY
}

int rtc_read(void) {  
  return NRF_RTC0->CC[0];
}

void rtc_reset(void) {
  NRF_CLOCK->TASKS_LFCLKSTOP = 1;
  NRF_CLOCK->TASKS_LFCLKCLEAR = 1;
  lfclk_config();
}
