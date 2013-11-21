#include "clock.h"
#include "nordic_common.h"
#include "app_timer.h"
#include "util.h"

static uint32_t 			total_minutes_past;
static app_timer_id_t       clock_timer_id;

uint32_t get_total_minutes_past(void) {
  return total_minutes_past;
}

static void on_timeout_handler(void * p_context) {
  UNUSED_PARAMETER(p_context);
  total_minutes_past++;

  mlog_println("on_timeout_handler", total_minutes_past);
}

void clock_init(void) {
  uint32_t err_code, one_minute, prescaler;
  total_minutes_past = 0;
  one_minute = 60 * 1000;
  prescaler = 0;

  // Create a repeating alarm that expires every minute
  err_code = app_timer_create(&clock_timer_id,
      APP_TIMER_MODE_REPEATED,
      on_timeout_handler);
  APP_ERROR_CHECK(err_code);

  app_timer_start(clock_timer_id, APP_TIMER_TICKS(one_minute, prescaler), NULL);
}
