#ifndef MOVING_AVG_h
#define MOVING_AVG_h
#include <stdint.h>
#include <string.h>


typedef struct {
  uint32_t avg;
  uint32_t sum;
  uint16_t oldest_sample;
  uint16_t* samples;
  uint16_t sample_size;
} samples_struct;

void moving_avg_init(samples_struct* s, int size);
void add_moving_average_sample(samples_struct* s, uint16_t value);

#endif
