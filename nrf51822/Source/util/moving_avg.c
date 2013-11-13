#include "moving_avg.h"

void moving_avg_init(samples_struct* s) {
  s->avg = 0;
  s->sum = 0;
  s->oldest_sample = 0;
  samples_init(s->samples);
}

void samples_init(uint16_t *_samples) {
  memset(_samples, 0, sizeof(_samples[0]) * SAMPLE_SIZE);
}

void add_moving_average_sample(samples_struct* s, uint16_t value) {
  uint16_t old_value = s->samples[s->oldest_sample];
  s->samples[s->oldest_sample] = value;
  s->sum = (s->sum - old_value) + value;
  s->avg = s->sum / SAMPLE_SIZE;

  if(s->oldest_sample < SAMPLE_SIZE - 1) {
    s->oldest_sample++;
  }
  else {
    s->oldest_sample = 0;
  }
}

