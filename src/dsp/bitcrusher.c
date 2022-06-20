#include <math.h>

#include "../lib/macros.h"

#include "dsp.h"
#include "bitcrusher.h"
#include "../ugen/ugen.h"

void
mono_bitcrush(FTYPE *L, dsp_state *state, FTYPE control)
{
  double QL = state->bitcrusher.quantized_bit_depth;
  *L = QL * ((int)(*L / QL));
}

void
dsp_set_bitcrusher_param(dsp_state *state, double quantized_bit_depth)
{
  state->bitcrusher.quantized_bit_depth = 2.0 / (pow(2.0, quantized_bit_depth) - 1.0);
}

DSP_callback
dsp_init_bitcrusher()
{
  DSP_callback cb = dsp_init();
  cb->fn_type = DSP_MONO_L;
  cb->fn_u.mono = mono_bitcrush;
  dsp_set_bitcrusher_param(&cb->state, 4.0);

  return cb;
}
