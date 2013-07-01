/*
 * Real Time Counter
 */ 

#include "nrf.h"

#define LFCLK_FREQUENCY           (32768UL)                 /*!< LFCLK frequency in Hertz, constant */
#define RTC_FREQUENCY             (8UL)                     /*!< required RTC working clock RTC_FREQUENCY Hertz. Changable */
#define COMPARE_COUNTERTIME       (3UL)                     /*!< Get Compare event COMPARE_TIME seconds after the counter starts from 0 */
#define COUNTER_PRESCALER         ((LFCLK_FREQUENCY/RTC_FREQUENCY) - 1)  /* f = LFCLK/(prescaler + 1) */

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
