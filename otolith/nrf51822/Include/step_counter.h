#ifndef STEP_COUNTER_h
#define STEP_COUNTER_h

#include "acc_driver.h"
#include "rtc.h"
#include "app_util.h"
#define SAMPLE_SIZE 100.0
#define SAMPLE_RATE 50.0
#define PI 3.14159265
#define X 0
#define Y 2
#define Z 4
#define GET_FIELD(acc_data_p, field) *((int16_t*)((char*)(acc_data_p) + field))
#define MAX_STEP_FREQ 5.0
#define MIN_STEP_FREQ 0.5
#define MIN_SAMPlES_BETWEEN  10 // ((1 / MAX_STEP_FREQ) * SAMPLE_RATE)
#define MIN_CONSECUTIVE_STEPS 4
#define MIN_PRECISION 30



typedef struct {
  int threshold;
  int max;
  int min;
  int axis;
  int precision;
	int interval;
	int total_steps;
	int temp_steps;
} measurements;

typedef struct {
	uint32_t start_time;
	uint32_t end_time;
	uint32_t steps;
} step_data;

struct _step_node {
  step_data data;
  struct _step_node * next;
}; 

typedef struct _step_node step_node;

int max_of(int a, int b);
int min_of(int a, int b);
void fake_acc_data_array(acc_data_t *acc_data_array, int size, float freq);
void filter(acc_data_t * acc_data_array, int size);
void print_acc_data_array(acc_data_t* acc_data_array, int size);
void print_measure_data(measurements* measure);
void set_acc_data(acc_data_t *data, int x, int y, int z);
int max_axis_offset(int dx, int dy, int dz);
void get_max_min(measurements *measure, acc_data_t *data, int size);
int count_steps(measurements *measure, acc_data_t *acc_data_array, int size);
int count_steps1(measurements *measure, acc_data_t *acc_data_array, int size);
void push_measurement (step_data data);
void store_stepCount(int steps);

int fill_data(acc_data_t* acc_array);

uint32_t get_step_count(void);

void step_counter_init(void);

/**@brief Inline function for encoding step_data.
 *
 * @param[in]   value            Value to be encoded.
 * @param[out]  p_encoded_data   Buffer where the encoded data is to be written.
 *
 * @return      Number of bytes written.
 */
static __INLINE uint8_t step_data_encode(step_data value, uint8_t * p_encoded_data)
{
  uint32_encode(value.start_time, p_encoded_data);
  uint32_encode(value.end_time, p_encoded_data + sizeof(uint32_t));
  uint32_encode(value.steps, p_encoded_data  + (2 * sizeof(uint32_t)));
  return (3 * sizeof(uint32_t));
}

#endif
