#ifndef PULSE_ANALYS_H
#define PuLSE_ANALYS_H

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
float get_avg_s02_AC_DC_ratio (void);
static inline float s02_abs(float num);
static inline float ac_dc_ratio_struct(so2_struct* s02_sample);
static inline float ac_dc_ratio(so2_d_type* dc, so2_d_type* ac);
#endif