#include "mwte_ffti.h"
#include <stdio.h>
#include <stdlib.h>

void print_cpx (num_cpx * num) {
  printf("r: %f im: %f\n", num->real, num->imag);
}
void inline mwte_fft_add(num_cpx* res, num_cpx a, num_cpx b) {
  res->real = a.real + b.real;
	res->imag = a.imag + b.imag;
}

void inline mwte_fft_add_to(num_cpx* res, num_cpx a) {
  mwte_fft_add(res, *res, a);
}

//initialized fft_state struct with default values
void mwte_fft_fft_state_init(fft_state* state) {
  state->data = NULL;
  state->d_len = 0;
  state->nfft = 0;
}

/*
// allocates struct for fft
// dlen = length of data
// nfft = number of indeces in FFT (for in place usualy size of data)
*/
void mwte_fft_alloc (int nfft, fft_state* state) {
  if(nfft > state->d_len || state->data == NULL) {
    state->data = (num_cpx*) malloc(sizeof(num_cpx) * nfft);
    state->d_len = nfft;
    state->nfft = nfft; 
  }
}

void mwte_fft_bit_reversal_sort(num_cpx* data, int length) {
  unsigned int i, swap_index, bit_length;
  bit_length = log2(length);

  for(i = 0; i < length; i++) {
    swap_index = mwte_fft_reverse_bits(i, bit_length);

    if(i <= swap_index) {
      mwte_fft_swap_indices(data, i, swap_index);
    }
  }
}

// packs a real d_type[d_len] array into the state num_cpx[d_len] data
void mwte_fft_pack_cpx (d_type* data, int d_len, fft_state* state) {
  if(state->data == NULL || state->d_len == 0) {
    state->data = (num_cpx*) malloc(sizeof(num_cpx) * d_len);
    state->d_len = d_len;
  }
  for (int i = 0; i < d_len; ++i)
  {
    state->data[i].real = data[i];
    state->data[i].real = 0;
  }
}

void inline mwte_fft_mul(num_cpx* res, num_cpx a, num_cpx b) {
  res->real = a.real * b.real - a.imag * b.imag;
  res->imag = a.real * b.imag + b.real * a.imag;
}

void inline mwte_fft_mul_eq(num_cpx* res, num_cpx a) {
	mwte_fft_mul(res, *res, a);
}

void inline mwte_fft_mul_scalar(num_cpx* res, d_type s) {
  res->real = res->real * s;
  res->imag = res->imag * s;
}

int inline mwte_fft_reverse_bits(unsigned int bits, unsigned int bit_length) {
  // Algorithm derived from
  // http://graphics.stanford.edu/~seander/bithacks.html
  unsigned int mask, bit_mask, adjusted_length, log, shift;

  // adjust bit length to be a power of 2
  // then shift it by the difference
  log = log2(bit_length);
  if(pow(2, log) != bit_length) {
    log++;
  }
  adjusted_length = pow(2, log);
  shift = adjusted_length - bit_length;

  mask = bit_mask = ~0;
  bit_mask ^= (mask << adjusted_length);

  while ((adjusted_length >>= 1)) {
    mask ^= (mask << adjusted_length);
    bits = ((bits >> adjusted_length) & mask) | ((bits << adjusted_length) & ~mask);
    bits &= bit_mask;
  }

  bits >>= shift;
  return bits;
}

void inline mwte_fft_sub(num_cpx* res, num_cpx a, num_cpx b) {
  res->real = a.real - b.real;
  res->imag = a.imag - b.imag;
}

void inline mwte_fft_sub_from(num_cpx* res, num_cpx a) {
  mwte_fft_sub(res, *res, a);
}

void inline mwte_fft_swap_indices(num_cpx* data, int i, int j) {
  num_cpx tmp = data[i];
  data[i] = data[j];
  data[j] = tmp;
}

void inline mwte_fft_w_index(num_cpx* data, d_type min_res, int index){
  data->real = cos(-1.0 * min_res * (d_type)index);
  data->imag = sin(-1.0 * min_res * (d_type)index);
}

void mwte_fft_in_place (fft_state* state) {
  int N0 = 2;
  int length = state->d_len;
  mwte_fft_bit_reversal_sort(state->data, length);
  while(N0 <= length) {
    mwte_sub_fft_in_place(state, N0);
    N0 *=2;
  }

}
static inline void mwte_sub_fft_in_place(fft_state* state, int N0) {
 
  num_cpx * fft_arr = state->data;
  num_cpx Wn ;
  int bfly_index0, bfly_index1; // indexes that the butterfly operartor will manipulate
  int group, index, offset, total_groups; 
  total_groups = state->d_len / N0; // total number of sub fft's equals size/(sub FFT size)
  d_type freq_res = (2.0*PI)/((d_type)N0);
  offset = N0/2;  // offset between index inputs to buttefly gate
  for(index = 0; index < offset; index++) {  // steps between indexes outside of groups to reduce Wn Computations
    mwte_fft_w_index(&Wn, freq_res, index);
    // iterate through each Sub FFT Group and calc butterfly for index and index+offset for group
    for(group = 0; group < total_groups; group++) { 
      bfly_index0 = group * N0 + index;
      bfly_index1 = group * N0 + index + offset;
      mwte_btfly2(&(fft_arr[bfly_index0]), &(fft_arr[bfly_index1]), &Wn); // calc 2 input butterfly
    }
  }
}

static inline void mwte_btfly2 (num_cpx * num_cpx0, num_cpx* num_cpx1, num_cpx * Wn) {
    num_cpx a = *num_cpx0;
    mwte_fft_mul_eq(num_cpx1, *Wn);
    mwte_fft_add_to(num_cpx0, *num_cpx1);
    mwte_fft_sub(num_cpx1,a, *num_cpx1);
}
