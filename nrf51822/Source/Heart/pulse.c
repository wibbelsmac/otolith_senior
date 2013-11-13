#include "pulse.h"
#include "kiss_fftr.h"
#include <math.h>

#define SAMPLE_FREQ 240
// #define SAMPLE_SIZE SAMPLE_FREQ * 4
#define SAMPLE_SIZE 1
#define SAMPLE_SIZE_FREQ SAMPLE_SIZE/2 + 1
#define GAIN 32

static kiss_fft_scalar sample_set[SAMPLE_SIZE];
static kiss_fft_cpx sample_set_freq[SAMPLE_SIZE_FREQ];
static uint16_t sample_set_index = 0;

void pulse_init(void) {
  
}

uint8_t add_pulse_sample(uint8_t ac, uint8_t v_ref) {
  if(sample_set_index < SAMPLE_SIZE) {    
    sample_set[sample_set_index] = v_ref * GAIN + ac;
		sample_set_index++;
    return 0;
  }
  else {
    sample_set_index = 0;
    return 1;
  }
}

uint16_t calculate_bpm() {
  kiss_fftr_cfg cfg = kiss_fftr_alloc(SAMPLE_SIZE, 0, NULL, NULL);
  kiss_fftr(cfg, sample_set, sample_set_freq);
  free(cfg);
  return get_max_freq();
}

uint16_t get_weighted_bpm(uint16_t index) {
  index = index - 1;
  float x = get_magnitude(sample_set_freq[index - 1].r ,sample_set_freq[index - 1].i);
  float y = get_magnitude(sample_set_freq[index].r, sample_set_freq[index].i);
  float z = get_magnitude(sample_set_freq[index + 1].r, sample_set_freq[index + 1].i);
  float avg = (x*(index - 1) + y*(index) + z*(index + 1)) / (x + y + z);
  
  uint16_t bpm = (avg / SAMPLE_SIZE_FREQ) * (SAMPLE_FREQ / 2) * 60;
  
  return bpm;
}

uint16_t get_max_freq() {
  uint16_t max_index = 1;
  uint16_t max_amp = 0;
  uint16_t i;
  uint16_t tmp;
  
  for(i = 1; i < SAMPLE_SIZE_FREQ; i++) {
    tmp = get_magnitude(sample_set_freq[i].r, sample_set_freq[i].i);
    if(max_amp < tmp) {
      max_amp = tmp;
      max_index = i;
    }
  }
  
  return get_weighted_bpm(i);
  
}

uint16_t get_magnitude(int16_t real, int16_t img) {
  return (uint16_t) sqrt((real*real) + (img*img));
}
