

#ifndef PULSE_h
#define PULSE_h
#include <stdint.h>
void pulse_init(void);
uint8_t add_sample(uint8_t ac, uint8_t v_ref);
uint16_t calculate_bpm(void);
uint16_t get_weighted_bpm(uint16_t index);
uint16_t get_max_freq(void);
uint16_t get_magnitude(int16_t real, int16_t img);
#endif