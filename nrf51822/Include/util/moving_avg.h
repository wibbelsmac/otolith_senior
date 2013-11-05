#ifndef MOVING_AVG_h
#define MOVING_AVG_h

#define SAMPLE_SIZE = 256;
typedef struct {
  uint32_t avg;
  uint32_t sum;
  uint16_t oldest_sample;
  uint16_t samples[SAMPLE_SIZE];
} samples_struct;

void moving_avg_init(samples_struct* s);
void samples_init(uint16_t *_samples);
void add_sample(samples_struct* s, uint16_t value);

#endif
