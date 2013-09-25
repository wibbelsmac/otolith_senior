/*
 * Real Time Counter
 */ 

#define LFCLK_FREQUENCY           (32768UL)                 /*!< LFCLK frequency in Hertz, constant */
#define RTC_FREQUENCY             (8UL)                     /*!< required RTC working clock RTC_FREQUENCY Hertz. Changable */
#define COMPARE_COUNTERTIME       (3UL)                     /*!< Get Compare event COMPARE_TIME seconds after the counter starts from 0 */
#define COUNTER_PRESCALER         ((LFCLK_FREQUENCY/RTC_FREQUENCY) - 1)  /* f = LFCLK/(prescaler + 1) */

void lfclk_config(void);

void rtc_config(void);

int rtc_read(void);

void rtc_stop(void);

void rtc_start(void);
