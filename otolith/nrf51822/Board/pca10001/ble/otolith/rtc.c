/*
 * Real Time Counter
 */ 

#include "nrf.h"
#include "rtc.h"

void lfclk_config(void) {
  NRF_CLOCK->LFCLKSRC = (CLOCK_LFCLKSRC_SRC_RC << CLOCK_LFCLKSRC_SRC_Pos);
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_LFCLKSTART = 1;
  while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0)
  {
  }
  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
}

void rtc_config(void) {
	//rtc_stop();
  NRF_RTC0->PRESCALER = COUNTER_PRESCALER;   // Set prescaler to a TICK of RTC_FREQUENCY
	rtc_start();
}

int rtc_read(void) {  
  return NRF_RTC0->COUNTER;
}

void rtc_stop(void) {
  NRF_RTC0->TASKS_STOP = 1;
  NRF_RTC0->TASKS_CLEAR = 1;
}

void rtc_start(void) {
	NRF_RTC0->TASKS_START = 1;
}

