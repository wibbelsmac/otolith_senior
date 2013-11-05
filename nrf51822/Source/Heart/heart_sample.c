#include "moving_avg.h"

#define SAMPLE_SIZE = 256;
typedef struct {
  uint32_t avg;
  uint32_t sum;
  uint16_t oldest_sample;
  uint16_t samples[SAMPLE_SIZE];
} samples_struct;


// =================================

void moving_avg_init(samples_struct* s) {
  s->avg = 0;
  s->sum = 0;
  s->oldest_sample = 0;
  samples_init(s->samples);
}

void samples_init(uint16_t *_samples) {
  memset(_samples, 0, sizeof(_samples[0]) * SAMPLE_SIZE);
}

void add_sample(samples_struct* s, uint16_t value) {
  uint16_t old_value = s->samples[oldest_sample];
  s->samples[oldest_sample] = value;
  s->sum = (sum - old_value) + value;
  s->avg = sum / SAMPLE_SIZE;

  if(oldest_sample < SAMPLE_SIZE - 1) {
    s->oldest_sample++;
  }
  else {
    s->oldest_sample = 0;
  }
}

