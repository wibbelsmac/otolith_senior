#include "adc.h"
#include "clock.h"
#include "dac_driver.h"
#include "pulse.h"
#include "math.h"
#include "util.h"
#include "nordic_common.h"
#include "ble_oto.h"
#include "pulse_analys.h"
#include <stdlib.h>


#define MIN_INDEX 3//(40 * (SAMPLE_SIZE_FREQ - 1))/((SAMPLE_FREQ * 60)/2) + 1
#define MAX_INDEX 14//(200 * (SAMPLE_SIZE_FREQ - 1))/((SAMPLE_FREQ * 60)/2) + 1




static      int   node_count;
static heart_node *           head;
static fft_state state;
// otolith service struct
static ble_oto_t *            otolith_service;
static int pulse_index;

num_cpx * sample_set = NULL;
static uint16_t sample_set_index = 0;

uint8_t add_pulse_sample(uint8_t ac, uint8_t v_ref) {
  if(sample_set_index < SAMPLE_SIZE) {
    state.data[sample_set_index].real = (d_type) (v_ref * GAIN + ac);
		state.data[sample_set_index].imag = 0;
		sample_set_index++;
    return 0;
  }
  else {
    sample_set_index = 0;
    return 1;
  }
}

uint16_t calculate_bpm() {
	mwte_fft_in_place(&state);
  return get_max_freq();
}

uint16_t get_weighted_bpm(uint16_t index) {
	float w, p, x;
  if(index <= 3) {
    w = 0.0f;
    p = 0.0f;
  } else {
    w = get_magnitudef(state.data[index - 2].real ,state.data[index - 2].imag);
   // p = get_magnitudef(state.data[index + 2].real ,state.data[index + 2].imag);
  }
	if(index <= 3 ){ // prevents dc polution of BPM calculation
    x = 0;
	} else {
		
    x = get_magnitudef(state.data[index - 1].real ,state.data[index - 1].imag);
  }
	float y = get_magnitudef(state.data[index].real, state.data[index].imag);
  float z = get_magnitudef(state.data[index + 1].real, state.data[index + 1].imag);
  p = get_magnitudef(state.data[index + 2].real, state.data[index + 2].imag);
  float avg = (w*(index - 2 )+ x*(index - 1) + y*(index) + z*(index + 1) + p*(index + 2)) / (w + x + y + z + p);
  mlog_num(avg);

//  uint16_t bpm = ((avg) * 60.0f * SAMPLE_FREQ) / ((2.00f) * (SAMPLE_SIZE_FREQ-1.0f));
//	float avg, index_avg;
//	int i;
//	int max = (index + index - MIN_INDEX > max) ? MAX_INDEX : (index + index - MIN_INDEX);
//	for(i = MIN_INDEX; i <= max; i++) {
//		float temp = get_magnitudef(state.data[i].real, state.data[i].imag);
//		avg +=temp;
//		index_avg += i * temp;
//	}
	
//	avg = index_avg / avg;
	uint16_t bpm = ((avg) * 60.0f * SAMPLE_FREQ) / ((2.00f) * (SAMPLE_SIZE_FREQ-1.0f));
  return bpm;
}

uint16_t get_max_freq() {
  uint16_t max_freq_index = MIN_INDEX;
  float max_amp = 0;
  uint16_t i;
  float tmp;

  for(i = MIN_INDEX; i <= MAX_INDEX; i++) {
	  tmp = get_magnitudef(state.data[i].real, state.data[i].imag);
    if(max_amp < tmp) {
      max_amp = tmp;
      max_freq_index = i;
    }
  }
  pulse_index = max_freq_index;
  mlog_println("Max index:", max_freq_index);
  return get_weighted_bpm(max_freq_index);
  
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
  heart_node * temp = (heart_node*) malloc(sizeof(heart_node));
  if(temp == NULL) {
    mlog_println("ERROR heart_node malloc returned NULL node_count: ", pls_get_measurement_count());
		return;
  }
  temp->data = data;
  temp->next = head;
  head = temp;
  node_count++;

  //if(sync_heart_info) {
    //ble_oto_send_heart_info(otolith_service);
  //}
}

void pls_build_sync_node (heart_data * status) {
  status->status = 1 << 31;
  status->start_time = get_total_minutes_past();
  status->end_time = get_total_minutes_past();
  status->bpm = 0;
  status->so2_sat = 0;
}

static void pls_initialize(void) {
  head = NULL;
  node_count = 0;
}

static void print_csv() {
  mlog_str("\r\n");
	int i;
  for(i = 0; i < SAMPLE_SIZE; i++) {
    mlog_num((uint16_t)state.data[i].real);
    mlog_str(",");
    mlog_num((uint16_t)state.data[i].imag);
    mlog_str("\r\n");
  }
  mlog_str("\r\n");
}

static void print_cpx_mag_csv() {
  mlog_str("\r\n");
  int i;
  for(i = 0; i < SAMPLE_SIZE; i++) {
    float value = calc_mag(&(state.data[i]));
    mlog_num((uint16_t)value);
    mlog_str("\r\n");
  }
  mlog_str("\r\n");
}

void pulse_init(ble_oto_t * _otolith_service) {
  otolith_service = _otolith_service;
	mwte_fft_fft_state_init(&state);
	mwte_fft_alloc(SAMPLE_SIZE, &state);
  s02_init(NUM_BEATS, SAMPLE_FREQ, BEAT_SAMPLE_LEN, MIN_BTWN_BEAT);
	mlog_str("Malloc\r\n");
  if(state.data) {
    mlog_str("malloc returned NULL\r\n");
  }
	mlog_str("finished pulse_init Malloc\r\n");
  pls_initialize();
	mlog_str("finished pls_initialize\r\n");
	dac_init();
	mlog_str("finished dac_init\r\n");
	adc_config();
	mlog_str("finished adc_config\r\n");
}

heart_data build_heart_data(uint16_t bpm, uint16_t so2_sat) {
  heart_data hd_struct;
  hd_struct.status = 1<<30;
  hd_struct.start_time = get_total_minutes_past();
  hd_struct.end_time = get_total_minutes_past();
  hd_struct.bpm = bpm;
  hd_struct.so2_sat = so2_sat;
	return hd_struct;
}

inline void pls_get_measurements(void) {
//print_csv();
  uint16_t bpm = calculate_bpm();
  mlog_println("BPM: ", bpm);
	
  uint16_t so2 = get_avg_s02_AC_DC_ratio();
  //mlog_println("so2: ", so2);
  //mlog_println("BPM so2_estimate: ", (int)get_bpm_estimate());
//print_cpx_mag_csv();
  pls_push_measurement(build_heart_data(bpm, so2), true);
  s02_reset();
}
uint16_t calculate_sa02_sat (){
  double dc = get_magnitudef(state.data[0].real, state.data[0].imag);
  double ac = get_magnitudef(state.data[pulse_index].real, state.data[pulse_index].imag);//sum(state.data, MIN_INDEX, MAX_INDEX);
  return (uint16_t) ((ac/dc) * 1000);
}

inline double sum (num_cpx* arr, int start, int end) {
  double temp = get_magnitudef(arr[start].real, arr[start].imag);
  for (int i = start; i <= end; ++i)
  {
    temp = temp + get_magnitudef(arr[i].real, arr[i].imag);
  }
  return temp;
}

void reset_measurement_count(void) {
  sample_set_index = 0;
}
