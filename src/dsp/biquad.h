#ifndef BIQUAD_H
#define BIQUAD_H

#include "../lib/macros.h"

typedef enum {
  BIQUAD_DIRECT,
  BIQUAD_CANONICAL,
  BIQUAD_TRANSPOSE_DIRECT,
  BIQUAD_TRANSPOSE_CANONICAL
} biquad_algorithm;

typedef enum {
  BIQUAD_a0,
  BIQUAD_a1,
  BIQUAD_a2,
  BIQUAD_b1,
  BIQUAD_b2,
  BIQUAD_c0,
  BIQUAD_d0,
  BIQUAD_NUM_COEFF
} biquad_filter_coeff;

typedef enum {
  BIQUAD_x_z1,
  BIQUAD_x_z2,
  BIQUAD_y_z1,
  BIQUAD_y_z2,
  BIQUAD_NUM_STATES
} biquad_state_array_indexes;

typedef struct biquad_param_t {
  double coeffs[BIQUAD_NUM_COEFF];
  double states[BIQUAD_NUM_STATES];
  biquad_algorithm alg;
} biquad_params;

void biquad_set_param(biquad_params *params, biquad_algorithm alg);

// called by filters
FTYPE biquad_process_sample(biquad_params *params, FTYPE sample);
FTYPE biquad_get_G(biquad_params *params);
FTYPE biquad_get_S(biquad_params *params);

#endif
