#include "pulse_analys.h"
#include <stdlib.h>
#include <stdio.h>

static so2_struct* s02_arr;
static int s02_arr_len;
static int sample_since_last_pulse;
static int num_so2_maxes;
static int min_btwn;
static int s02_arr_index;

void s02_init(int num_beats, int sample_freq, int beat_sample_len, int min_btwn_beat) {
	s02_arr =  (so2_struct*)malloc(sizeof(so2_struct) * num_beats);
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
int s02_add_sample (so2_d_type dc, so2_d_type ac, int samp_index) {
	int index = s02_arr_index % s02_arr_len;
	int last_index = (s02_arr_index-1) % s02_arr_len;
	sample_since_last_pulse++;
	if((sample_since_last_pulse >= min_btwn) && (num_so2_maxes < s02_arr_len)) {  // greater than max distance between samples
		s02_arr[index].ac = ac;
		s02_arr[index].dc = dc;
		s02_arr[index].index = samp_index;
		num_so2_maxes++;
		s02_arr_index++;
		sample_since_last_pulse = 0;
	} else if((sample_since_last_pulse < min_btwn) && (num_so2_maxes != 0) && // less than max distance, but ac:dc greater than last 
			 ac_dc_ratio_struct(&(s02_arr[last_index])) < ac_dc_ratio(&dc,&ac)) {
		s02_arr[last_index].ac = ac;
		s02_arr[last_index].dc = dc;
		s02_arr[index].index = samp_index;
		sample_since_last_pulse = 0;
	}
	
	
	return 	(num_so2_maxes >= s02_arr_len); // return true if room for more samples
}

float get_avg_s02_AC_DC_ratio (void) {
	float ac_sum = 0;
	int i;
	for(i = 0; i < num_so2_maxes; i++) {
		printf("Average of index%d is: %f\n", i, ac_dc_ratio_struct(&(s02_arr[i])));
		ac_sum += ac_dc_ratio_struct(&(s02_arr[i]));
	}
	if(num_so2_maxes > 0)
		return ac_sum / num_so2_maxes;
	else
		return 0.0f;
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
