typedef __fp16 so2_d_type;
typedef struct  {
so2_d_type ac;
so2_d_type dc;
int index;	
} so2_struct;
static so2_struct* so2_arr;
static int so2_arr_len;
static int sample_since_last_pulse;
static int num_so2_maxes;
static int min_btwn;
static int so2_arr_index;
void s02_init(int num_beats, int sample_freq, int beat_sample_len, int min_btwn_beat);
void s02_reset(void);
// returns true if num_so2_maxes < so2_arr_len
int s02_add_sample (so2_d_type* dc, so2_d_type* ac, int samp_index); 
float get_s02_AC_DC_ratio (void);
float get_bpm_estimate (void);
float get_avg_s02_AC_DC_ratio (void);
static inline float s02_abs(float num);
static inline float ac_dc_ratio_struct(so2_struct* s02_sample);
static inline float ac_dc_ratio(so2_d_type* dc, so2_d_type* ac);


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
