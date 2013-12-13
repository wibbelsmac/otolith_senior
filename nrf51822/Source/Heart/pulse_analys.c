#include "pulse_analys.h"

#include "adc.h"
#include "dac_driver.h"
#include "clock.h"
#include "util.h"
#include "moving_avg.h"
#include <stdio.h>
#include <stdlib.h>

#define THRESHOLD  .75
#define MIN_THRESHOLD .50f
#define MAX_THRESHOLD 1.25f
#define MAX_ADC_READ 0 //9000
#define DIFF_MIN_AMPL_DIFF 20 // 78mV 3.3V @ 8bit res 
#define NUM_DIFF_SAMPLES 10
// state variables
static bool found_diff0 = 0;
static bool found_diff1 = 0;

static so2_d_type diff0 = 0;
static so2_d_type diff1 = 0;
static so2_d_type temp_diff = 0;

static int lmin0 = -1;
static int lmax0 = -1;
static int lmin1 = -1;
static int lmax1 = -1;

static so2_d_type min0;
static so2_d_type max0;
static so2_d_type min1;
static so2_d_type max1;

static int sample_index = 0;
static int node_count;
// linked list
static heart_node* head;
static ble_oto_t*  otolith_service;
static samples_struct s02_moving_avg;
static samples_struct diff_moving_avg;

heart_data build_heart_data(uint16_t bpm, uint16_t so2_sat) {
  heart_data hd_struct;
  hd_struct.status = 1<<30;
  hd_struct.start_time = get_total_minutes_past();
  hd_struct.end_time = get_total_minutes_past();
  hd_struct.bpm = (uint16_t) bpm;
  hd_struct.so2_sat = so2_sat;
	return hd_struct;
}

void check_min_and_max(so2_d_type* ac, int samp_index) {
  if(!found_diff0) {
    set_min_and_max(&lmax0, &lmin0, &max0, &min0, ac, samp_index);
  }
  else {
    set_min_and_max(&lmax1, &lmin1, &max1, &min1, ac, samp_index);
  }
}

bool compare_diff() {
  set_diff_state();

  if(found_diff0 && found_diff1) {
    //    we found two peaks
    //    if the peaks are of the same magnitude then
    //      store the values
    //      reset state
    // mlog_str("Found Two Peaks\r\n");
    // mlog_print("Min0: ", min0); mlog_println(" at index: ", lmin0);
    // mlog_print("Max0: ", max0); mlog_println(" at index: ", lmax0);
    // mlog_println("Diff0: ", diff0);
    // mlog_print("Min1: ", min1); mlog_println(" at index: ", lmin1);
    // mlog_print("Max1: ", max1); mlog_println(" at index: ", lmax1);
    // mlog_println("Diff1: ", diff1);
    // mlog_print("Diff0: ", max0 - min0); mlog_println(" Index_diff: ", lmax0 - lmin0);
    // mlog_print("Diff1: ", max1 - min1); mlog_println(" Index_diff: ", lmax1 - lmin1);
    //mlog_print("Min_index_diff: ", lmin1 - lmin0); mlog_println(" Max_index_diff: ", lmax1 - lmax0);
    mlog_println("DIFF BPM: ", 60.0f * 120.0f /  (float)(lmax1 - lmax0));
    return true;
  }

  return false;
}

int compare_peaks() {
  if(diff0 > diff1) {
    return compare_peak_threshold(&diff0, &diff1);
  }

  if(!compare_peak_threshold(&diff1, &diff0)) {
    // diff0 is too small, set diff1 to diff0
    shift_diff1();
    return false;
  }

  return true;
}

bool compare_peak_threshold(so2_d_type* big_diff, so2_d_type* little_diff) {
  // need to compare the peaks with a threshold + and - the original peak
 // mlog_println("MIN: ", MIN_THRESHOLD * (*big_diff));
 // mlog_println("MAX: ", MAX_THRESHOLD * (*big_diff));
  so2_d_type min_threshold_diff = MIN_THRESHOLD * (*big_diff);
  so2_d_type max_threshold_diff = MAX_THRESHOLD * (*big_diff);

  if((*little_diff) >= min_threshold_diff && (*little_diff) <= max_threshold_diff) {
    return true;
  }

  return false;
}

void diff_add_sample(so2_d_type* dc, so2_d_type* ac) {
  check_min_and_max(ac, sample_index);
  set_diff(ac);
  // mlog_str("IN diff_add_sample\r\n");
  //mlog_println("State: ", found_diff1<<1 | found_diff0<<1);
  if(compare_diff()) {
    /* do whatever we are going to do with the two peaks */
    float temp_bpm = get_bpm();
    if(temp_bpm > 35 && temp_bpm < 200) {
      add_moving_average_sample(&diff_moving_avg, temp_bpm);
      add_moving_average_sample(&s02_moving_avg, (uint16_t)(1000.0f - (((*ac) * 1000.0f) / (*dc))));
      if(diff_get_measurement_count() < 50) {
        diff_push_node();
      }
      mlog_println("BPM_AVG: ", diff_moving_avg.avg);
      mlog_println("s02_AVG: ", s02_moving_avg.avg);
    }
    shift_diff1();  
  }
  
  sample_index++;
}

void diff_build_sync_node (heart_data * status) {
  status->status = 1 << 31;
  status->start_time = get_total_minutes_past();
  status->end_time = get_total_minutes_past();
  status->bpm = 0;
  status->so2_sat = 0;
}

int diff_get_measurement_count(void) {
  return node_count;
}

static void diff_initialize(void) {
  head = NULL;
  node_count = 0;
moving_avg_init(&s02_moving_avg, NUM_DIFF_SAMPLES); 
 moving_avg_init(&diff_moving_avg, NUM_DIFF_SAMPLES); 
}

void diff_pulse_init(ble_oto_t * _otolith_service) {
  otolith_service = _otolith_service;
  diff_initialize();
	mlog_str("finished diff_initialize\r\n");
	dac_init();
	mlog_str("finished dac_init\r\n");
	adc_config();
	mlog_str("finished adc_config\r\n");
}

void diff_push_node() {
  float bpm =  60.0f * 120.0f /  (float)(lmax1 - lmax0);
  heart_data heart = build_heart_data(diff_moving_avg.avg, s02_moving_avg.avg);
  diff_push_measurement(heart);
}

int diff_pop_measurement(heart_data * data) {
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

void diff_push_measurement(heart_data data) {
  heart_node * temp = (heart_node*) malloc(sizeof(heart_node));
  if(temp == NULL) {
    mlog_println("ERROR heart_node malloc returned NULL node_count: ", diff_get_measurement_count());
		return;
  }
  temp->data = data;
  temp->next = head;
  head = temp;
  node_count++;
}

float get_bpm (void) {
  return  60.0f * 120.0f /  (float)(lmax1 - lmax0);
}

bool is_diff_found(so2_d_type* diff) {
  // mlog_println("diff: ", *diff);
  // mlog_println("temp_diff: ", temp_diff);
  // mlog_println("result: ", temp_diff < (THRESHOLD * ((float)(*diff))));
  return (temp_diff < (THRESHOLD * ((float)(*diff))) && ((*diff) > DIFF_MIN_AMPL_DIFF));
}

bool is_index_set(int* index) {
  return ((*index) >= 0);
}

void reset_state () {
  lmin0 = -1;
  lmax0 = -1;
  lmin1 = -1;
  lmax1 = -1;

  found_diff0 = 0;
  found_diff1 = 0;

  diff0 = 0;
  diff1 = 0;

  min0 = MAX_ADC_READ;
  min1 = MAX_ADC_READ;
  max0 = 0;
  max1 = 0;
}

// if diff is greater update appropriate diff1 or diff2 
// always update temp diff if min is set
void set_diff(so2_d_type* ac) {
  set_temp_diff(ac);
  //mlog_println("State: ", (found_diff0<<1 + found_diff1));
  if((!found_diff0) && temp_diff > diff0) {
    // diff0 is not set and temp diff higher, set it
    diff0 = temp_diff;
    // mlog_println("Lmin0: ", lmin0);
    // mlog_println("Lmax0: ", lmax0);
  }
  else if (found_diff0 && temp_diff > diff1) {
    // diff0 is set and temp diff higher, set it
    diff1 = temp_diff;
    // mlog_println("Lmin1: ", lmin1);
    // mlog_println("Lmax1: ", lmax1);
  }
}

void set_diff_state() {
  if(!found_diff0 && is_index_set(&lmin0) && is_index_set(&lmax0)) {
    found_diff0 = is_diff_found(&diff0);
    if(found_diff0)
    mlog_str("FOUND diff0\r\n");
  }
  else if (found_diff0 && is_index_set(&lmin1) && is_index_set(&lmax1)){
    found_diff1 = is_diff_found(&diff1) && compare_peaks();
    if(found_diff1)
    mlog_str("FOUND diff1\r\n");
  }
}

void set_min_and_max(int* max_index, int* min_index, so2_d_type* current_max, so2_d_type* current_min, so2_d_type* ac, int samp_index) {
  if(should_set_min(min_index, current_min, ac)) {
    // Set min
    *current_min = *ac;
    *min_index = samp_index;

    // reset max
    *current_max = 0;
    *max_index = -1;
  }
  else if(should_set_max(max_index, min_index, current_max, ac)) {
    // Set max
    *current_max = *ac;
    *max_index = samp_index;
  }
}

void set_temp_diff(so2_d_type* ac) {
  if (found_diff0) {
    // if first peak set and second peak set
    temp_diff = (*ac) - min1; // update temp diff with min 1
  }
  else {
    // if diff1 is not set update with min 0
    temp_diff = (*ac) - min0; // update temp diff with min 0
    //mlog_println("min0: ", min0);
    //mlog_println("max0: ", max0);
    //mlog_println("temp_diff: ", temp_diff);
  }
}

void shift_diff1() {
  diff0 = diff1;
  min0 = min1;
  max0 = max1;
  lmin0 = lmin1;
  lmax0 = lmax1;

  diff1 = 0;
  min1 = MAX_ADC_READ;
  max1 = 0;
  lmin1 = -1;
  lmax1 = -1;
  found_diff1 = 0;
}

// if min_index is set && max_index is not set or new value greater, return true
bool should_set_max(int* max_index, int* min_index, so2_d_type* current_max, so2_d_type* ac) {
  return (is_index_set(min_index) && (!is_index_set(max_index)) || (*ac) > (*current_max));
}

// if min_index is not set or new value less than current_min, return true
bool should_set_min(int* min_index, so2_d_type* current_min, so2_d_type* ac) {
  return ((!is_index_set(min_index)) || (*ac) < (*current_min));
}

