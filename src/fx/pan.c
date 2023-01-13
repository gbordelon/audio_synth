#include <stdio.h>
#include <string.h>

#include "../lib/macros.h"

#include "pan.h"
#include "fx.h"

extern FX_unit fx_unit_head;

/*
 * accept L, R, C signals from a single parent then apply the control signal to pan L and R
 * also copy the parent control signal because why not?
 */
void
fx_unit_pan_process_frame(fx_unit_idx idx)
{
#define dst (fx_unit_head[idx].output_buffer.lrc)
#define src (fx_unit_head[fx_unit_head[idx].parents[0]].output_buffer.lrc)
  memcpy(dst, src, 3 * sizeof(FTYPE));
  dst[FX_L] *= (1.0 - dst[FX_C]);
  dst[FX_R] *= dst[FX_C];
#undef src
#undef dst
}

void
fx_unit_pan_cleanup(FX_unit_state state)
{
  // do nothing
}

void
fx_unit_pan_set_params(FX_unit_state state, FX_unit_params params)
{
  // do nothing
}

void
fx_unit_pan_reset(FX_unit_state state, FX_unit_params params)
{
  state->sample_rate = params->sample_rate;
  fx_unit_pan_set_params(state, params);
}

fx_unit_idx
fx_unit_pan_init(FX_unit_params params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_pan_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_pan_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_pan_reset;

  fx_unit_pan_reset(&fx_unit_head[idx].state, params);
  return idx;
}
