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

FX_compound_unit
fx_compound_unit_pan_init(FX_unit_params pan_p, FX_unit_params sig_p)
{
  FX_compound_unit rv = fx_compound_unit_init(4, 1);
  // panner
  fx_unit_idx pan = fx_unit_pan_init(pan_p);
  // control joiner
  fx_unit_params cj_p = fx_unit_control_joiner_default();
  fx_unit_idx cj = fx_unit_control_joiner_init(&cj_p);
  // signal genenerator
  // control signal generator has no parent
  fx_unit_idx sg = fx_unit_signal_source_init(sig_p);

  // passthru
  fx_unit_params pt_p = fx_unit_passthru_default();
  fx_unit_idx pt = fx_unit_passthru_init(&pt_p);

  rv->units[0] = pt;
  rv->units[1] = cj;
  rv->units[2] = sg;
  rv->units[3] = pan;

  // panner has control joiner as parent
  fx_unit_parent_ref_add(pan, cj);
  // control joiner has signal generator as control parent
  fx_unit_parent_ref_add(cj, pt);
  fx_unit_parent_ref_add(cj, sg);

  rv->heads[0] = pt;

  // panner is output for compound obj
  rv->tail = pan;

  return rv;
}

fx_unit_params
fx_unit_pan_default()
{
  fx_unit_params params = {0};
  params.sample_rate = DEFAULT_SAMPLE_RATE;
  params.t = FX_UNIT_PAN;

  return params;
}

