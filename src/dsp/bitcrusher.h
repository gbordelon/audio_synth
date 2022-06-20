#ifndef DSP_BITCRUSHER_H
#define DSP_BITCRUSHER_H

#include "../lib/macros.h"

#include "dsp.h"

/*
 * Using code from Designing Audio Effect Plugins in C++ by Prikle, pgs. 550:551
 */

void mono_bitcrush(FTYPE *L, dsp_state *state, FTYPE control);

void dsp_set_bitcrusher_param(dsp_state *state, double quantized_bit_depth);

#endif
