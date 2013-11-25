
#ifndef SYNC_TIMER_H__
#define SYNC_TIMER_H__

#include <stdint.h>
#include "app_timer.h"

static void sync_timeout_handler(void * p_context);
uint32_t sync_timer_init(user_alarm_evt_handler_t evt_handler);


static app_timer_id_t            sync_timer_id;

#endif
