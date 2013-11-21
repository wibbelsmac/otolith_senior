#include "pulse.h"
#include "kiss_fftr.h"
#include "math.h"
#include "util.h"
#include "nordic_common.h"
#include "ble_oto.h"
#define SAMPLE_FREQ 120
// #define SAMPLE_SIZE SAMPLE_FREQ * 4
#define SAMPLE_SIZE 256
#define SAMPLE_SIZE_FREQ SAMPLE_SIZE/2 + 1
#define GAIN 32
#define MAX_INDEX (200 * (SAMPLE_SIZE_FREQ - 1))/3600 + 1

// 
static      int   node_count;
static heart_node *           head;                      
// Timer attributes
static app_timer_id_t        step_timer_id;
static uint32_t              total_minutes_past;


// otolith service struct
static ble_oto_t *            otolith_service;


kiss_fft_scalar * sample_set;
kiss_fft_cpx * sample_set_freq;
uint16_t sample_set_index = 0;




static void on_timeout_handler(void * p_context) {
  UNUSED_PARAMETER(p_context);
  total_minutes_past++;

  mlog_println("on_timeout_handler", total_minutes_past);
}

static void init_pulse_timer() {
  uint32_t err_code, one_minute, prescaler;
  total_minutes_past = 0;
  one_minute = 60 * 1000;
  prescaler = 0;

  // Create a repeating alarm that expires every minute
  err_code = app_timer_create(&step_timer_id,
      APP_TIMER_MODE_REPEATED,
      on_timeout_handler);
  APP_ERROR_CHECK(err_code);

  app_timer_start(step_timer_id, APP_TIMER_TICKS(one_minute, prescaler), NULL);
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
  //index = index - 1;
  float x = get_magnitudef(sample_set_freq[index - 1].r ,sample_set_freq[index - 1].i);
  float y = get_magnitudef(sample_set_freq[index].r, sample_set_freq[index].i);
  float z = get_magnitudef(sample_set_freq[index + 1].r, sample_set_freq[index + 1].i);
  float avg = (x*(index - 1) + y*(index) + z*(index + 1)) / (x + y + z);
  mlog_num(avg);
  uint16_t bpm = ((avg - 2.0f) * 60.0f * (SAMPLE_FREQ / 2.00f)) / (SAMPLE_SIZE_FREQ-1.0f);
  
  return bpm;
}

uint16_t get_max_freq() {
  uint16_t max_index = 2;
  float max_amp = 0;
  uint16_t i;
  float tmp;

  for(i = 2; i <= MAX_INDEX; i++) {
	  tmp = get_magnitudef(sample_set_freq[i].r, sample_set_freq[i].i);
    if(max_amp < tmp) {
      max_amp = tmp;
      max_index = i;
    }
  }
  mlog_println("Max index:", max_index);
  return get_weighted_bpm(max_index);
  
}

uint16_t get_magnitude(int16_t real, int16_t img) {
  return (uint16_t) sqrt((double)((real*real) + (img*img)));
}

float get_magnitudef(int16_t real, int16_t img) {
  return sqrt((double)((real*real) + (img*img)));
}




int pls_get_measurement_count(void) {
  return node_count;
}

int pls_pop_measurement (heart_data * data) {
  if(head != NULL) {
    heart_node *temp = head->next;
    *data = head->data;
    free(head);
    head = temp;
    node_count--;
    return 0;
  }
  return 1;
}

void pls_push_measurement(heart_data data, bool sync_heart_info) {
  heart_node * temp = malloc(sizeof(heart_node));
  temp->data = data;
  temp->next = head;
  head = temp;
  node_count++;

  if(sync_heart_info) {
    ble_oto_send_heart_info(otolith_service);
  }
}

void pls_push_sync_node (void) {
  heart_data status;
  status.status = 1 << 31;
  status.start_time = total_minutes_past;
  status.end_time = total_minutes_past;
  status.bpm = 0;
  status.so2_sat = 0;
  pls_push_measurement(status, false);
}

static void pls_initialize(void) {
  head = malloc(sizeof(heart_node));
  node_count = 0;
}



void pulse_init(ble_oto_t * _otolith_service) {
  otolith_service = _otolith_service;
  sample_set = malloc(sizeof(kiss_fft_scalar) * SAMPLE_SIZE);
  sample_set_freq =  malloc(sizeof(kiss_fft_cpx) * SAMPLE_SIZE_FREQ);

  if(sample_set_freq == NULL) {
    mlog_str("malloc returned NULL\r\n");
  }
  init_pulse_timer();
  pls_initialize();
}

heart_data build_heart_data(uint16_t bpm, uint16_t so2_sat) {
  heart_data hd_struct;
  hd_struct.status = 1;
  hd_struct.start_time = total_minutes_past;
  hd_struct.end_time = total_minutes_past;
  hd_struct.bpm = bpm;
  hd_struct.so2_sat = so2_sat;
	return hd_struct;
}

void pls_get_measurements(void) {
  uint16_t bpm = calculate_bpm();
  uint16_t so2 = 97;  //calculate_sow();
  pls_push_measurement(build_heart_data(bpm, so2), true);
}
