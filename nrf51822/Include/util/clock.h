#ifndef CLK_H__
#define CLK_H__
#include "nrf.h"
#include "nrf51_bitfields.h"
uint32_t get_total_minutes_past(void);
void clock_init(void);

#endif