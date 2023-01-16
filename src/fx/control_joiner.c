#include <stdio.h>
#include <string.h>

#include "../lib/macros.h"

#include "control_joiner.h"
#include "fx.h"

extern FX_unit fx_unit_head;

/*
 */
void
fx_unit_control_joiner_process_frame(fx_unit_idx idx)
{
#define dst (fx_unit_head[idx].output_buffer.lrc)
#define src1 (fx_unit_head[fx_unit_head[idx].parents[0]].output_buffer.lrc)
#define src2 (fx_unit_head[fx_unit_head[idx].parents[1]].output_buffer.lrc)
  memcpy(dst, src1, 2 * sizeof(FTYPE));
  dst[FX_C] = src2[FX_C];
#undef src2
#undef src1
#undef dst
}

void
fx_unit_control_joiner_cleanup(FX_unit_state state)
{
  // do nothing
}

void
fx_unit_control_joiner_set_params(FX_unit_state state, FX_unit_params params)
{
  // do nothing
}

void
fx_unit_control_joiner_reset(FX_unit_state state, FX_unit_params params)
{
  state->sample_rate = params->sample_rate;
  fx_unit_control_joiner_set_params(state, params);
}

fx_unit_idx
fx_unit_control_joiner_init(FX_unit_params params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_control_joiner_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_control_joiner_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_control_joiner_reset;

  fx_unit_control_joiner_reset(&fx_unit_head[idx].state, params);
  return idx;
}

fx_unit_params
fx_unit_control_joiner_default()
{
  fx_unit_params cj = {0};
  cj.sample_rate = DEFAULT_SAMPLE_RATE;
  cj.t = FX_UNIT_CONTROL_JOINER;
  return cj;
}
