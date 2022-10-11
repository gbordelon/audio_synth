#ifndef DSP_BITCRUSHER_H
#define DSP_BITCRUSHER_H

#include "../lib/macros.h"

typedef struct bitcrusher_params_t {
  FTYPE quantized_bit_depth;
} bitcrusher_params;

#include "dsp.h"

void dsp_set_bitcrusher_param(dsp_state *state, bitcrusher_params params);

#endif
