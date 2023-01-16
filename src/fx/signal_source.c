#include <stdio.h>
#include <string.h>

#include "../lib/macros.h"

#include "../ugen/dfo.h"
#include "../ugen/ugen.h"

#include "signal_source.h"
#include "fx.h"

extern FX_unit fx_unit_head;

void
fx_unit_signal_source_process_frame(fx_unit_idx idx)
{
#define triphase (fx_unit_head[idx].state.u.signal_source.ugen_triphase)
  // switch on fx_unit_head[idx].state.u.signal_source.t to get value
  FTYPE val;
  switch (fx_unit_head[idx].state.u.signal_source.t) {
  case FX_SIGNAL_UGEN:
    ugen_sample_fast_triphase(fx_unit_head[idx].state.u.signal_source.u.ugen, 0.0, triphase);
    val = triphase[UGEN_PHASE_NORM];
    break;
  case FX_SIGNAL_DFO:
    val = dfo_sample(fx_unit_head[idx].state.u.signal_source.u.dfo);
    break;
  case FX_SIGNAL_CONSTANT:
    // fall through
  default:
    val = fx_unit_head[idx].state.u.signal_source.u.constant; 
    break;
  };
#undef triphase

#define dst (fx_unit_head[idx].output_buffer.lrc)
#define bitmask (fx_unit_head[idx].state.u.signal_source.d)
  // use bitmask to decide which fields get the value
  if (bitmask & FX_SIGNAL_L) {
    dst[FX_L] = val;
  }
  if (bitmask & FX_SIGNAL_R) {
    dst[FX_R] = val;
  }
  if (bitmask & FX_SIGNAL_C) {
    dst[FX_C] = val;
  }
#undef bitmask
#undef dst
}

void
fx_unit_signal_source_cleanup(FX_unit_state state)
{
  switch (state->u.signal_source.t) {
  case FX_SIGNAL_UGEN:
    ugen_cleanup(state->u.signal_source.u.ugen);
    break;
  case FX_SIGNAL_DFO:
    dfo_cleanup(state->u.signal_source.u.dfo);
    break;
  case FX_SIGNAL_CONSTANT:
    // fall through
  default:
    // do nothing
    break;
  };
}

void
fx_unit_signal_source_set_params(FX_unit_state state, FX_unit_params params)
{
  state->u.signal_source.t = params->u.signal_source.t;
  state->u.signal_source.d = params->u.signal_source.d;
  memcpy(&state->u.signal_source.u, &params->u.signal_source.u, sizeof(state->u.signal_source.u));
}

void
fx_unit_signal_source_reset(FX_unit_state state, FX_unit_params params)
{
  state->sample_rate = params->sample_rate;
  switch (state->u.signal_source.t) {
  case FX_SIGNAL_UGEN:
    ugen_set_sample_rate(state->u.signal_source.u.ugen, params->sample_rate);
    break;
  case FX_SIGNAL_DFO:
    dfo_set_sample_rate(state->u.signal_source.u.dfo, params->sample_rate);
    break;
  case FX_SIGNAL_CONSTANT:
    // fall through
  default:
    // do nothing
    break;
  };
  fx_unit_signal_source_set_params(state, params);
}

fx_unit_idx
fx_unit_signal_source_init(FX_unit_params params)
{
  fx_unit_idx idx = fx_unit_init();
  fx_unit_head[idx].state.t = params->t;
  fx_unit_head[idx].state.f.cleanup = fx_unit_signal_source_cleanup;
  fx_unit_head[idx].state.f.process_frame = fx_unit_signal_source_process_frame; 
  fx_unit_head[idx].state.f.reset = fx_unit_signal_source_reset;

  fx_unit_signal_source_reset(&fx_unit_head[idx].state, params);
  return idx;
}

fx_unit_params
signal_source_default_helper()
{
  fx_unit_params params = {0};
  params.sample_rate = DEFAULT_SAMPLE_RATE;
  params.t = FX_UNIT_SIGNAL_SOURCE;
  params.u.signal_source.d = FX_SIGNAL_C;

  return params;
}

fx_unit_params
fx_unit_signal_source_constant_default()
{
  fx_unit_params params = signal_source_default_helper();
  params.u.signal_source.t = FX_SIGNAL_CONSTANT;
  params.u.signal_source.u.constant = 0.5;

  return params;
}

fx_unit_params
fx_unit_signal_source_dfo_default()
{
  fx_unit_params params = signal_source_default_helper();
  params.u.signal_source.t = FX_SIGNAL_DFO;
  params.u.signal_source.u.dfo = dfo_init(DEFAULT_SAMPLE_RATE);
  dfo_set_freq(params.u.signal_source.u.dfo, 0.2);
  dfo_set_scale(params.u.signal_source.u.dfo, 0.0, 1.0);

  return params;
}

fx_unit_params
fx_unit_signal_source_ugen_default()
{
  fx_unit_params params = signal_source_default_helper();
  params.u.signal_source.t = FX_SIGNAL_UGEN;
  params.u.signal_source.u.ugen = ugen_init_tri(0.2, DEFAULT_SAMPLE_RATE);
  //ugen_set_cr(params.u.signal_source.u.ugen);
  ugen_set_scale(params.u.signal_source.u.ugen, 0.5, 1.0);

  return params;
}
