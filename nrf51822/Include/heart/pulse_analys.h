#ifndef PULSE_ANALYS_H
#define PuLSE_ANALYS_H

typedef struct {
	uint32_t status;
	uint32_t start_time;
	uint32_t end_time;
	uint16_t bpm;
	uint16_t so2_sat;
} heart_data;

struct _heart_node {
  heart_data data;
  struct _heart_node * next;
};

typedef struct _heart_node heart_node;

heart_data build_heart_data(float bpm, uint16_t so2_sat);
void check_min_and_max(so2_d_type* ac, int samp_index);
bool compare_diff(void);
int compare_peaks(void);
bool compare_peak_threshold(so2_d_type* big_diff, so2_d_type* little_diff);
void diff_add_sample(so2_d_type* dc, so2_d_type* ac);
void diff_build_sync_node (heart_data * status);
int diff_get_measurement_count(void);
static void diff_initialize(void);
void diff_pulse_init(ble_oto_t * _otolith_service);
void diff_push_node(void);
int diff_pop_measurement(heart_data * data);
void diff_push_measurement(heart_data data);
bool is_diff_found(so2_d_type* diff);
bool is_index_set(int* index);
void reset_state(void);
void set_diff(so2_d_type* ac);
void set_diff_state(void);
void set_min_and_max(int* max_index, int* min_index, so2_d_type* current_max, so2_d_type* current_min, so2_d_type* ac, int samp_index);
void set_temp_diff(so2_d_type* ac);
void shift_diff1(void);
bool should_set_max(int* max_index, int* min_index, so2_d_type* current_max, so2_d_type* ac);
bool should_set_min(int* min_index, so2_d_type* current_min, so2_d_type* ac);

#endif
