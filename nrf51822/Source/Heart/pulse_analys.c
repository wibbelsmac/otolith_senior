#include "pulse_analys.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>

static so2_struct* s02_arr;
static int s02_arr_len;
static int sample_since_last_pulse;
static int num_so2_maxes;
static int min_btwn;
static int s02_arr_index;
static int sample_freq;

void s02_init(int num_beats, int t_sample_freq, int beat_sample_len, int min_btwn_beat) {
  s02_arr =  (so2_struct*)malloc(sizeof(so2_struct) * num_beats);
  if(s02_arr == NULL) {
    mlog_str("ERROR s02_malloc returned NULL");
  }
  sample_freq = t_sample_freq;
  s02_arr_len = num_beats;
  sample_since_last_pulse = min_btwn_beat * 3 / 4; // dont want to pick up beat halfway off window
  num_so2_maxes = 0;
  s02_arr_index = 0;
  min_btwn = min_btwn_beat;
}

void s02_reset(void) {
  sample_since_last_pulse = min_btwn * 3 / 4; // dont want to pick up beat halfway off window
  num_so2_maxes = 0;
  s02_arr_index = -1;
}
// returns true if num_so2_maxes > s02_arr_len
int s02_add_sample (so2_d_type* dc, so2_d_type* ac, int samp_index) {  
  int index = s02_arr_index % s02_arr_len;
  int last_index = (s02_arr_index-1) % s02_arr_len;
  sample_since_last_pulse++;
  if((sample_since_last_pulse >= min_btwn) && (num_so2_maxes < s02_arr_len)) {  // greater than max distance between samples
    s02_arr[index].ac = *ac;
    s02_arr[index].dc = *dc;
    s02_arr[index].index = samp_index;
    num_so2_maxes++;
    s02_arr_index++;
    sample_since_last_pulse = 0;
  } else if((sample_since_last_pulse < min_btwn) && (num_so2_maxes != 0) && // less than max distance, but ac:dc greater than last 
       ac_dc_ratio_struct(&(s02_arr[last_index])) < ac_dc_ratio(dc,ac)) {
    s02_arr[last_index].ac = *ac;
    s02_arr[last_index].dc = *dc;
    s02_arr[index].index = samp_index;
    sample_since_last_pulse = 0;
  }
  
  
  return  (num_so2_maxes >= s02_arr_len); // return true if room for more samples
}

float get_avg_s02_AC_DC_ratio (void) {
  double ac_sum = 0;
  double dc_sum = 0;
  int i;
  for(i = 0; i < num_so2_maxes; i++) {
    ac_sum += s02_arr[i].ac;
    dc_sum += s02_arr[i].dc;
  }
  if(num_so2_maxes > 0)
    return (1000.0f - ((ac_sum * 1000.0f) / dc_sum));
  else
    return 0.0f;
}

float get_bpm_estimate (void) {
  float avg_sample_diff = 0;
  float avg_max = 0;
  int last_index, i;
  if(num_so2_maxes > 2) {
    last_index = s02_arr[0].index;
  } else {
    return 0;
  }
  for (int i = 1; i < num_so2_maxes -1; ++i)
  {
    avg_max += s02_arr[i].ac;
  }
  avg_max = avg_max / (num_so2_maxes -2);
  for(i = 1; i < num_so2_maxes - 1; i++) {
    if(s02_arr[i].index > (avg_max * .75f))
    mlog_println("Adding diff: ", s02_arr[i].index - last_index); 
    avg_sample_diff += s02_arr[i].index - last_index;
    last_index = s02_arr[i].index;
  }
  mlog_println("Avg diff: ", (int) avg_sample_diff/(num_so2_maxes - 2) * 1000);
  mlog_println("sample_freq: ", sample_freq);
  float tmp =  (60 / ((avg_sample_diff / ((((float)num_so2_maxes) -2))) * (1/((float)sample_freq))));  // Return average diff between beats* 60/f_s / num_maxes 
  mlog_println("BPM_est: ", tmp);
  return tmp;
}

float inline s02_abs(float num) {
  if(num < 0)
    return -1.0f * (num); 
  else
    return num;
}
float inline ac_dc_ratio_struct(so2_struct* s02_sample) {
  return s02_abs(s02_sample->ac) / s02_sample->dc;
}

float inline ac_dc_ratio(so2_d_type* dc, so2_d_type* ac) {
  return s02_abs(*ac) / *dc;
}


#define THRESHOLD  .75
#define MIN_THRESHOLD .50f
#define MAX_THRESHOLD 1.25f
#define MAX_ADC_READ 0 //9000
#define DIFF_MIN_AMPL_DIFF 20 // 78mV 3.3V @ 8bit res 
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
// linked list
static heart_node* head;
static ble_oto_t*  otolith_service;

typedef __fp16 so2_d_type;
typedef struct  {
so2_d_type lmin0;
so2_d_type lmax0;
so2_d_type lmin1;
so2_d_type lmax1;
int index;
} beat_struct;

bool is_index_set(int* index) {
  return ((*index) >= 0);
}

bool is_diff_found(so2_d_type* diff) {
  // mlog_println("diff: ", *diff);
  // mlog_println("temp_diff: ", temp_diff);
  // mlog_println("result: ", temp_diff < (THRESHOLD * ((float)(*diff))));
  return (temp_diff < (THRESHOLD * ((float)(*diff))) && ((*diff) > DIFF_MIN_AMPL_DIFF));
}

// if min_index is not set or new value less than current_min, return true
bool should_set_min(int* min_index, so2_d_type* current_min, so2_d_type* ac) {
  return ((!is_index_set(min_index)) || (*ac) < (*current_min));
}

// if min_index is set && max_index is not set or new value greater, return true
bool should_set_max(int* max_index, int* min_index, so2_d_type* current_max, so2_d_type* ac) {
  return (is_index_set(min_index) && (!is_index_set(max_index)) || (*ac) > (*current_max));
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

void check_min_and_max(so2_d_type* ac, int samp_index) {
  if(!found_diff0) {
    set_min_and_max(&lmax0, &lmin0, &max0, &min0, ac, samp_index);
  }
  else {
    set_min_and_max(&lmax1, &lmin1, &max1, &min1, ac, samp_index);
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


uint16_t get_bpm() {
  return 1;
}

int diff_pop_measurement (heart_data * data) {
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

void diff_push_measurement(heart_data data, bool sync_heart_info) {
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

void diff_build_sync_node (heart_data * status) {
  status->status = 1 << 31;
  status->start_time = get_total_minutes_past();
  status->end_time = get_total_minutes_past();
  status->bpm = 0;
  status->so2_sat = 0;
}

static void diff_initialize(void) {
  head = NULL;
  node_count = 0;
}


void diff_add_sample(so2_d_type* dc, so2_d_type* ac) {
  check_min_and_max(ac, sample_index);
  set_diff(ac);
  if(compare_diff()) {
    /* do whatever we are going to do with the two peaks */
    shift_diff1();
  }

  sample_index++;
}


void pulse_init(ble_oto_t * _otolith_service) {
  otolith_service = _otolith_service;
  s02_init(NUM_BEATS, SAMPLE_FREQ, BEAT_SAMPLE_LEN, MIN_BTWN_BEAT);

  diff_initialize();
	mlog_str("finished diff_initialize\r\n");
	dac_init();
	mlog_str("finished dac_init\r\n");
	adc_config();
	mlog_str("finished adc_config\r\n");
}

