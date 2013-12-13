#include "moving_avg.h"
#include "util.h"
#include <stdlib.h>


void moving_avg_init(samples_struct* s, int size) {
  s->avg = 0;
  s->sum = 0;
  s->oldest_sample = 0;
	s->sample_size = size;
  s->samples = (uint16_t*) malloc(sizeof(uint16_t) * s->sample_size);
  memset(s->samples, 0, sizeof(uint16_t) * s->sample_size);
}

void add_moving_average_sample(samples_struct* s, uint16_t value) {
  uint16_t old_value = s->samples[s->oldest_sample];
 
  s->samples[s->oldest_sample] = value;
  s->sum = (s->sum - old_value) + value;
  s->avg = s->sum / s->sample_size;

  if(s->oldest_sample < s->sample_size - 1) {
    s->oldest_sample++;
  }
  else {
    s->oldest_sample = 0;
  }
}

