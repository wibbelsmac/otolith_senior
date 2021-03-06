

#ifndef PULSE_h
#define PULSE_h
#include <stdint.h>
#include "ble_oto.h"
#include "mwte_ffti.h"

#define SAMPLE_FREQ 120
//#define SAMPLE_FREQ 60
#define SAMPLE_SIZE 512
#define SAMPLE_SIZE_FREQ SAMPLE_SIZE/2 + 1
#define GAIN 32

#define NUM_BEATS  12
#define BEAT_SAMPLE_LEN  (60 * SAMPLE_SIZE_FREQ) / 40 // max length of heartbeat
#define MIN_BTWN_BEAT  ((int)(((float)SAMPLE_FREQ) / (220.0f / 60.0f)))  // min seperation between heart beats


//typedef struct {
//	uint32_t status;
//	uint32_t start_time;
//	uint32_t end_time;
//	uint16_t bpm;
//	uint16_t so2_sat;
//} heart_data;
//
//struct _heart_node {
//  heart_data data;
//  struct _heart_node * next;
//}; 


//typedef struct _heart_node heart_node;


uint8_t add_pulse_sample(uint8_t ac, uint8_t v_ref);
uint16_t calculate_bpm(void);
uint16_t get_weighted_bpm(uint16_t index);
uint16_t get_max_freq(void);
uint16_t get_magnitude(int16_t real, int16_t img);
float get_magnitudef(int16_t real, int16_t img);
void pls_get_measurements(void);
int pls_pop_measurement (heart_data * data);
void pls_push_measurement(heart_data data, bool sync_heart_info);
int pls_get_measurement_count(void);
void pls_build_sync_node(heart_data * data);
void pulse_init(ble_oto_t * _otolith_service);
heart_data build_heart_data(uint16_t bpm, uint16_t so2_stat);
double sum (num_cpx* arr, int start, int end);
uint16_t calculate_sa02_sat (void);
void reset_measurement_count(void);
static void print_cpx_mag_csv();
#endif
