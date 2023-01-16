#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../lib/macros.h"

#include "bitcrusher.h"
#include "fx.h"

extern FX_unit fx_unit_head;

/*
 * Using code from Designing Audio Effect Plugins in C++ by Pirkle, pgs. 550:551
 */
void
fx_unit_bitcrusher_process_frame(fx_unit_idx idx)
{
#define dst (fx_unit_head[idx].output_buffer.lrc)
#define src (fx_unit_head[fx_unit_head[idx].parents[0]].output_buffer.lrc)
#define QL (fx_unit_head[idx].state.u.bitcrusher.quantized_bit_depth)
  dst[FX_L] = QL * ((int)(src[FX_L] / QL));
  dst[FX_R] = QL * ((int)(src[FX_R] / QL));
  dst[FX_C] = src[FX_C];
#undef QL
#undef src
#undef dst
}

void
fx_unit_bitcrusher_cleanup(FX_unit_state state)
{
  // do nothing
}

void
fx_unit_bitcrusher_set_params(FX_unit_state state, FX_unit_params params)
{
  state->u.bitcrusher.quantized_bit_depth = 2.0 / (pow(2.0, params->u.bitcrusher.quantized_bit_depth) - 1.0);
}

void
fx_unit_bitcrusher_reset(FX_unit_state state, FX_unit_params params)
{
  state->sample_rate = params->sample_rate;
  fx_unit_bitcrusher_set_params(state, params);
}

fx_unit_idx
fx_unit_bitcrusher_init(FX_unit_params params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_bitcrusher_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_bitcrusher_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_bitcrusher_reset;

  fx_unit_bitcrusher_reset(&fx_unit_head[idx].state, params);
  return idx;
}

FX_compound_unit
fx_compound_unit_bitcrusher_init(FX_unit_params params)
{
  FX_compound_unit rv = fx_compound_unit_init(1, 1);
  fx_unit_idx bitcrusher = fx_unit_bitcrusher_init(params);

  rv->units[0] = bitcrusher;
  rv->heads[0] = bitcrusher;
  rv->tail = bitcrusher;

  return rv;
}

fx_unit_params
fx_unit_bitcrusher_default()
{
  fx_unit_params params = {0};
  params.sample_rate = DEFAULT_SAMPLE_RATE;
  params.t = FX_UNIT_BITCRUSHER;
  params.u.bitcrusher.quantized_bit_depth = 6.0;
  return params;
}
