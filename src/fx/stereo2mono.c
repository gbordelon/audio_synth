#include <stdio.h>
#include <string.h>

#include "../lib/macros.h"

#include "stereo2mono.h"
#include "fx.h"

extern FX_unit fx_unit_head;

/*
 * down mix stereo to mono with a -3dB scaling
 */
void
fx_unit_s2m_process_frame(fx_unit_idx idx)
{
#define dst (fx_unit_head[idx].output_buffer.lrc)
#define src (fx_unit_head[fx_unit_head[idx].parents[0]].output_buffer.lrc)
#define state (fx_unit_head[idx].state.u.s2m)
  if (state.left) {
    dst[FX_L] = 0.707 * (src[FX_L] + src[FX_R]);
  } else {
    dst[FX_R] = 0.707 * (src[FX_L] + src[FX_R]);
  }

  dst[FX_C] = src[FX_C]; // control pass-thru
#undef state
#undef src
#undef dst
}

void
fx_unit_s2m_cleanup(FX_unit_state state)
{
  // do nothing
}

void
fx_unit_s2m_set_params(FX_unit_state state, FX_unit_params params)
{
  state->u.s2m.left = params->u.s2m.left;
}

void
fx_unit_s2m_reset(FX_unit_state state, FX_unit_params params)
{
  state->sample_rate = params->sample_rate;
  fx_unit_s2m_set_params(state, params);
}

fx_unit_idx
fx_unit_s2m_init(FX_unit_params params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_s2m_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_s2m_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_s2m_reset;

  fx_unit_s2m_reset(&fx_unit_head[idx].state, params);
  return idx;
}

fx_unit_params
fx_unit_s2m_default()
{
  fx_unit_params params = {0};
  params.sample_rate = DEFAULT_SAMPLE_RATE;
  params.t = FX_UNIT_STEREO_2_MONO;
  params.u.s2m.left = true;
  return params;
}
