
#include "user_alarm.h"
#include "nordic_common.h"
#include "sync_timer.h"
#include "util.h"
#include "main.h"
#include "step_counter.h"
#include "pulse_analys.h"
#include "adc.h"

#define MINUTE_IN_TICKS          APP_TIMER_TICKS(60*1000, 0)
#define DAY_IN_MINUTES           24*60;

static app_timer_id_t            m_user_alarm_timer_id;
static user_alarm_evt_handler_t  m_evt_handler;
static uint16_t                  m_remaining_minutes;

static void sync_timeout_handler(void * p_context)
{
  mlog_str("sync_timout_handler\r\n");
  if(get_measurement_count() > 1) {
    sync_steps();
  } else {
    mlog_println("Step Nodes ", get_measurement_count());
  }
	 if(diff_get_measurement_count() > 1) {
    sync_hearts();
  } else {
    mlog_println("Heart Nodes ", diff_get_measurement_count());
  }
}

uint32_t sync_timer_init(user_alarm_evt_handler_t evt_handler)
{
    
    // Create a repeating alarm that expires every minute
    uint32_t err = app_timer_create(&sync_timer_id,
                          APP_TIMER_MODE_REPEATED,
                          sync_timeout_handler);
    mlog_println("App_timer_error: ", err);
    // Stop current timer (although it may not even be running)
    app_timer_stop(sync_timer_id);
     
    // Start timer
    app_timer_start(sync_timer_id, 1000*150, NULL);
	
		return 0;

}