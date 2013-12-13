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


#define THRESHHOLD  (7 / 8) 

static int diff1 = -1;
static int diff2 = -1;
static int temp_diff = -1;

static int lmin1 = -1;
static int lmax1 = -1;
static int lmin2 = -1;
static int lmax2 = -1;

static so2_d_type min1;
static so2_d_type max1;
static so2_d_type min2;
static so2_d_type max2;

typedef __fp16 so2_d_type;
typedef struct  {
so2_d_type lmin0;
so2_d_type lmax0;
so2_d_type lmin1;
so2_d_type lmax1;
int index;  
} beat_struct;

void set_min_and_max(so2_d_type* ac, int samp_index) {
 if(lmin1 < 0 ||  (*ac) < min1 ) {   // if lmin not set or new value greater, set it
    min1 = *ac;
    lmin1 = samp_index;
  } else if (lmin1 > 0 && lmax1 < 0 ||  (*ac) > max1) { // if min1 set && lmax1 not set or new value grater set it
    max1 = *ac;
    lmax1 = samp_index;
  } else if(lmin1 > 0 && lmax1 > 0) { // if First diff already found
     if(lmin2 < 0 ||  (*ac) < min2 ) {   // if lmin2 not set or new value greater, set it
        min2 = *ac;
        lmin2 = samp_index;
      } else if (lmin2 > 0 && lmax2 < 0 ||  (*ac) > max2) { // if min2 set && lmax2 not set or new value grater set it
        max2 = *ac;
        lmax2 = samp_index;
      }
  }
}
void set_diff(so2_d_type* ac) {
  if(lmin2 < 0 && lmin1 > 0) {  // if 2nd peak not set and first peak set
      temp_diff = *ac - min1; // update temp diff with min 1
  } else if (lmin2 > 0 && lmin1 > 0) { // if first peak set and second peak set
    temp_diff = *ac - min2; // update temp diff with min 2
  }
  if((diff1 < 0 || temp_diff > diff1)) { // diff 0 not set or temp diff higher set it
    diff1 = temp_diff;
  } else if ((diff1 > 0 && (diff2 < 0 || temp_diff > diff2))) { // if diff 0 set and diff2 not set or lower reset it
    diff2 = temp_diff;
  }
}
// not exactely sure what to do here
bool compare_diff() {
  if(lmin1 > 0 && lmin2 > 0 && temp_diff < THRESHHOLD * diff2) {
    /* WE HAVE FOUND TWO PEAKS */
    mlog_str("Found Two Peaks");
    return true;
  }
  return false;
}
void reset_lmin_lmax () {
  lmin1 = -1;
  lmax1 = -1;
  lmin2 = -1;
  lmax2 = -1;
}

void diff_add_sample(so2_d_type* dc, so2_d_type* ac, int samp_index) {
  set_min_and_max(ac, samp_index);
  set_diff(ac); // if diff is greater update appropriate diff1 or diff2 always update temp diff is min is set
  if(compare_diff()) {  // if temp diff 
    /* do whatever we are going to do wiht the two peaks */
    reset_lmin_lmax();
  }
}


