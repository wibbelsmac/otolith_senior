#ifndef MWTE_FFT_IN_PLACE_H
#define MWTE_FFT_IN_PLACE_H


#include <limits.h>
#include <math.h>
//#include <fplib.h>

#define PI 3.14159265359
typedef __fp16 d_type;


typedef struct  {
d_type real;
d_type imag;	
} num_cpx;

typedef struct {
	int nfft;
	int d_len;	// length of data
	num_cpx* data;
} fft_state;




// res = a + b
static inline void mwte_fft_add(num_cpx* res, num_cpx a, num_cpx b);
// res += a
static inline void mwte_fft_add_to(num_cpx* res, num_cpx a);

//initialized fft_state struct with default values
void mwte_fft_fft_state_init(fft_state* state);
// allocates struct for fft
// dlen = length of data
// nfft = number of indeces in FFT (for in place usualy size of data)
void mwte_fft_alloc (int nfft, fft_state* state);
// sorts data by the bit reversal length must be power of 2
void mwte_fft_bit_reversal_sort(num_cpx* data, int length);
// packs a real d_type[d_len] array into the state num_cpx[d_len] data
void mwte_fft_pack_cpx (d_type* data, int d_len, fft_state* state);
// res = a * b
static inline void mwte_fft_mul(num_cpx* res, num_cpx a, num_cpx b);
// res *= a
static inline void mwte_fft_mul_eq(num_cpx* res, num_cpx a);
// res = a * s
static inline void mwte_fft_mul_scalar(num_cpx* res, d_type* s);
// reverses bits up to given bit_length, bit_length must be power of two
int mwte_fft_reverse_bits(unsigned int bits, unsigned int bit_length);
// res = a - b
static inline void mwte_fft_sub(num_cpx* res, num_cpx a, num_cpx b);
// res -= a
static inline void mwte_fft_sub_from(num_cpx* res, num_cpx a);
// swaps data[i] and data[j]
void mwte_fft_swap_indices(num_cpx* data, int i, int j);
// F_r = sum[k=0:k=N0-1](f_k * W_N0^kr)
// W_N0 = e^(-2.0 * pi / N0)
// k = index
// data is set to the complex number representation of W_N0^index
static inline void mwte_fft_w_index(num_cpx* data, d_type* min_res, int index);
// performs forward fft in-place
void mwte_fft_in_place (fft_state* state);
static inline void mwte_sub_fft_in_place(fft_state* state, int N0);
//static inline void mwte_btfly2 (num_cpx * num_cpx0, num_cpx* num_cpx1, num_cpx * Wn);
static inline void mwte_btfly2 (num_cpx * num_cpx0, num_cpx* num_cpx1, num_cpx * Wn);
#endif
