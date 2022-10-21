#include <math.h>

#include "../lib/macros.h"

#include "dsp.h"
#include "bitcrusher.h"
#include "../ugen/ugen.h"

/*
 * Using code from Designing Audio Effect Plugins in C++ by Pirkle, pgs. 550:551
 */

FTYPE
stereo_bitcrush(FTYPE *L, FTYPE *R, dsp_state *state, FTYPE control)
{
  FTYPE QL = state->bitcrusher.quantized_bit_depth;
  *L = QL * ((int)(*L / QL));
  *R = QL * ((int)(*R / QL));
  return control;
}

void
dsp_set_bitcrusher_param(dsp_state *state, bitcrusher_params params)
{
  state->bitcrusher.quantized_bit_depth = 2.0 / (pow(2.0, params.quantized_bit_depth) - 1.0);
}

DSP_callback
dsp_init_bitcrusher()
{
  bitcrusher_params params = { 4.0 };
  DSP_callback cb = dsp_init();
  dsp_set_stereo(cb, stereo_bitcrush);
  dsp_set_bitcrusher_param(&cb->state, params);

  return cb;
}
